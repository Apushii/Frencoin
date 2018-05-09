// Copyright (c) 2017 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//#include <amount.h>
//#include <base58.h>
#include <assets/assets.h>
#include <assets/assetdb.h>
//#include <rpc/server.h>
//#include <script/standard.h>
//#include <utilstrencodings.h>

#include "amount.h"
#include "base58.h"
#include "chain.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "httpserver.h"
#include "validation.h"
#include "net.h"
#include "policy/feerate.h"
#include "policy/fees.h"
#include "policy/policy.h"
#include "policy/rbf.h"
#include "rpc/mining.h"
#include "rpc/safemode.h"
#include "rpc/server.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "wallet/coincontrol.h"
#include "wallet/feebumper.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"


//issue(to_address, asset_name, qty, units=1, reissuable=false)
//Issue an asset with unique name. Unit as 1 for whole units, or 0.00000001 for satoshi-like units. Qty should be whole number. Reissuable is true/false for whether additional units can be issued by the original issuer.
UniValue issue(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 7)
        throw std::runtime_error(
            "issue \"asset-name\" qty \"address\" ( units ) ( reissuable ) ( has_ipfs ) \"( ipfs_hash )\"\n"
            "\nIssue an asset with unique name.\n"
            "Unit as 1 for whole units, or 0.00000001 for satoshi-like units.\n"
            "Qty should be whole number.\n"
            "Reissuable is true/false for whether additional units can be issued by the original issuer.\n"

            "\nArguments:\n"
            "1. \"asset_name\"            (string, required) a unique name\n"
            "2. \"qty\"                   (integer, required) the number of units to be issued\n"
            "3. \"address\"               (string), required), address asset will be sent to, if it is empty, address will be generated for you\n"
            "4. \"units\"                 (integer, optional, default=1), the atomic unit size (1, 0.1, ... ,0.00000001)\n"
            "5. \"reissuable\"            (boolean, optional, default=false), whether future reissuance is allowed\n"
            "6. \"has_ipfs\"              (boolean, optional, default=false), whether ifps hash is going to be added to the asset\n"
            "7. \"ipfs_hash\"             (string, optional but required if has_ipfs = 1), an ipfs hash\n"

            "\nResult:\n"
            "\"txid\"                     (string) The transaction id\n"

            "\nExamples:\n"
            + HelpExampleCli("issue", "\"myaddress\" \"myassetname\" 1000")
            + HelpExampleCli("issue", "\"myaddress\" \"myassetname\" 1000 \"0.0001\"")
            + HelpExampleCli("issue", "\"myaddress\" \"myassetname\" 1000 \"0.01\" true")
        );


    ObserveSafeMode();
    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::string asset_name = request.params[0].get_str();

    CAmount nAmount = AmountFromValue(request.params[1]);

    std::string address = request.params[2].get_str();

    if (address != "") {
        CTxDestination destination = DecodeDestination(address);
        if (!IsValidDestination(destination)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Raven address: ") + address);
        }
    } else {
        // Create a new address
        std::string strAccount;

        if (!pwallet->IsLocked()) {
            pwallet->TopUpKeyPool();
        }

        // Generate a new key that is added to wallet
        CPubKey newKey;
        if (!pwallet->GetKeyFromPool(newKey)) {
            throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
        }
        CKeyID keyID = newKey.GetID();

        pwallet->SetAddressBook(keyID, strAccount, "receive");

        address = EncodeDestination(keyID);
    }

    int units = 1;
    if (request.params.size() > 3)
        units = request.params[3].get_int();
    bool reissuable = false;
    if (request.params.size() > 4)
        reissuable = request.params[4].get_bool();

    bool has_ipfs = false;
    if (request.params.size() > 5)
        has_ipfs = request.params[5].get_bool();

    std::string ipfs_hash = "";
    if (request.params.size() > 6 && has_ipfs)
        ipfs_hash = request.params[6].get_str();

    int length = asset_name.size() / 8 + 1;
    CNewAsset asset(asset_name, nAmount, length, units, reissuable ? 1 : 0, has_ipfs ? 1 : 0, ipfs_hash);

    // Validate the assets data
    std::string strError;
    if (!asset.IsValid(strError, true)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }

    CAmount curBalance = pwallet->GetBalance();

    if (curBalance < Params().IssueAssetBurnAmount()) {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");
    }

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Get the script for the burn address
    CScript scriptPubKey = GetScriptForDestination(DecodeDestination(Params().IssueAssetBurnAddress()));

    CMutableTransaction mutTx;

    CWalletTx wtxNew;
    CCoinControl coin_control;

    // Create and send the transaction
    CReserveKey reservekey(pwallet);
    CAmount nFeeRequired;
    std::string strTxError;
    std::vector<CRecipient> vecSend;
    int nChangePosRet = -1;
    bool fSubtractFeeFromAmount = false;
    CRecipient recipient = {scriptPubKey, Params().IssueAssetBurnAmount(), fSubtractFeeFromAmount};
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransactionWithAsset(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet, strTxError, coin_control, asset, DecodeDestination(address))) {
        if (!fSubtractFeeFromAmount && Params().IssueAssetBurnAmount() + nFeeRequired > curBalance)
            strTxError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strTxError);
    }

    CValidationState state;
    if (!pwallet->CommitTransaction(wtxNew, reservekey, g_connman.get(), state)) {
        strTxError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strTxError);
    }

    UniValue result(UniValue::VARR);
    result.push_back(wtxNew.GetHash().GetHex());
    return result;
}

//getaddressbalances(address, minconf=1)
//Returns a list of all the asset balances for address in this node’s wallet, with at least minconf confirmations.
UniValue getaddressbalances(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1)
        throw std::runtime_error(
            "getaddressbalances \"address\" ( minconf )\n"
            "\nReturns a list of all the asset balances for address in this node's wallet, with at least minconf confirmations.\n"

            "\nArguments:\n"
            "1. \"address\"               (string, required) a raven address\n"
            "2. \"minconf\"               (integer, optional, default=1) the minimum required confirmations\n"

            "\nResult:\n"
            "TBD\n"

            "\nExamples:\n"
            + HelpExampleCli("getaddressbalances", "\"myaddress\"")
            + HelpExampleCli("getaddressbalances", "\"myaddress\" 5")
        );

    std::string address_ = request.params[0].get_str();
    CTxDestination destination = DecodeDestination(address_);
    if (!IsValidDestination(destination)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Raven address: ") + address_);
    }

    int minconf = 1;
    if (!request.params[1].isNull()) {
        minconf = request.params[1].get_int();
        if (minconf < 1) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid minconf: ") + std::to_string(minconf));
        }
    }

    UniValue result(UniValue::VARR);
    return result;
}


//getaddressbalances(address, minconf=1)
//Returns a list of all the asset balances for address in this node’s wallet, with at least minconf confirmations.
UniValue getallassets(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
                "getallassets\n"
                "\nReturns a list of all asset names\n"

                "\nResult:\n"
                "[ "
                "{ name : (string)\n"
                "  name_length : (number)\n"
                "  amount : (number)\n"
                "  units : (number)\n"
                "  reissuable : (number)\n"
                "  has_ipfs : (number)\n"
                "  ipfs_hash : (hash)}\n, only if has_ipfs = 1"
                "{...}, {...}\n"
                "]\n"

                "\nExamples:\n"
                + HelpExampleCli("getallassets", "")
                + HelpExampleCli("getallassets", "")
        );


    LOCK(cs_main);
    UniValue result (UniValue::VARR);
    if (passets)
        for (auto it : passets->setAssets) {
            UniValue value(UniValue::VOBJ);
            value.push_back(Pair("name: ", it.strName));
            value.push_back(Pair("name_length: ", it.nNameLength));
            value.push_back(Pair("amount: ", it.nAmount));
            value.push_back(Pair("units: ", it.units));
            value.push_back(Pair("reissuable: ", it.nReissuable));
            value.push_back(Pair("has_ipfs: ", it.nHasIPFS));
            if (it.nHasIPFS)
                value.push_back(Pair("ipfs_hash: ", it.strIPFSHash));

            result.push_back(value);
        }

    return result;
}

//getaddressbalances(address, minconf=1)
//Returns a list of all the asset balances for address in this node’s wallet, with at least minconf confirmations.
UniValue getmyassets(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
                "getmyassets\n"
                "\nReturns a list of all asset that are owned by this wallet\n"

                "\nResult:\n"
                "[ "
                "{ name : (string)\n"
                "  name_length : (number)\n"
                "  amount : (number)\n"
                "  units : (number)\n"
                "  reissuable : (number)\n"
                "  has_ipfs : (number)\n"
                "  ipfs_hash : (hash)}\n, only if has_ipfs = 1"
                "{...}, {...}\n"
                "]\n"

                "\nExamples:\n"
                + HelpExampleCli("getmyassets", "")
                + HelpExampleCli("getmyassets", "")
        );


    LOCK(cs_main);
    UniValue result (UniValue::VARR);
    if (passets) {
        UniValue assets(UniValue::VOBJ);
        for (auto it : passets->mapMyUnspentAssets) {
            UniValue outs(UniValue::VARR);
            for (auto out : it.second) {
                UniValue tempOut(UniValue::VOBJ);
                tempOut.push_back(Pair("txid", out.hash.GetHex()));
                tempOut.push_back(Pair("index", std::to_string(out.n)));
                outs.push_back(tempOut);
            }
            assets.push_back(Pair(it.first, outs));
            outs.clear();
        }
        result.push_back(assets);
    }

    return result;
}

// TODO Used to test database, remove before release
UniValue getassetaddresses(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
                "getassetsaddresses asset_name\n"
                "\nReturns a list of all address that own the given asset"

                "\nArguments:\n"
                "1. \"asset_name\"               (string, required) name of asset\n"

                "\nResult:\n"
                "[ "
                "address,\n"
                "address,\n"
                "address,\n"
                "...\n"
                "]\n"

                "\nExamples:\n"
                + HelpExampleCli("getassetsaddresses", "assetname")
                + HelpExampleCli("getassetsaddresses", "assetname")
        );

    LOCK(cs_main);

    std::string asset_name = request.params[0].get_str();

    if (!passets)
        return NullUniValue;

    if (!passets->mapAssetsAddresses.count(asset_name))
        return NullUniValue;

    UniValue addresses(UniValue::VARR);

    auto setAddresses = passets->mapAssetsAddresses.at(asset_name);
    for (auto it : setAddresses)
        addresses.push_back(it);

    return addresses;
}

// TODO Used to test database, remove before release
UniValue transfer(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 3)
        throw std::runtime_error(
                "transfer asset_name address amount\n"
                "\nReturns a list of all address that own the given asset"

                "\nArguments:\n"
                "1. \"asset_name\"               (string, required) name of asset\n"
                "2. \"address\"                  (string, required) address to send the asset to\n"
                "3. \"amount\"                   (number, required) number of assets you want to send to the address\n"

                "\nResult:\n"
                "txid"
                "[ \n"
                "txid\n"
                "]\n"

                "\nExamples:\n"
                + HelpExampleCli("transfer", "\"asset_name\" \"address\" \"20\"")
                + HelpExampleCli("transfer", "\"asset_name\" \"address\" \"20\"")
        );

    ObserveSafeMode();
    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::string asset_name = request.params[0].get_str();

    std::string address = request.params[1].get_str();

    CAmount nAmount = AmountFromValue(request.params[2]);

    if (!IsValidDestinationString(address))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Raven address: ") + address);

    if (!passets)
        throw JSONRPCError(RPC_DATABASE_ERROR, std::string("passets isn't initialized"));

    std::set<COutPoint> myAssetOutPoints;
    if (!passets->GetAssetsOutPoints(asset_name, myAssetOutPoints))
        throw JSONRPCError(RPC_INVALID_PARAMS, std::string("This wallet doesn't own any assets with the name: ") + asset_name);

    CAmount curBalance = pwallet->GetBalance();

    if (curBalance < Params().IssueAssetBurnAmount()) {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");
    }

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Get the script for the burn address
    CScript scriptPubKey = GetScriptForDestination(DecodeDestination(address));

    // Update the scriptPubKey with the transfer asset information
    CAssetTransfer assetTransfer(asset_name, nAmount);
    assetTransfer.ConstructTransaction(scriptPubKey);

    CMutableTransaction mutTx;

    CWalletTx wtxNew;
    CCoinControl coin_control;

    // Create and send the transaction
    CReserveKey reservekey(pwallet);
    CAmount nFeeRequired;
    std::string strTxError;
    std::vector<CRecipient> vecSend;
    int nChangePosRet = -1;
    bool fSubtractFeeFromAmount = false;
    CRecipient recipient = {scriptPubKey, 0, fSubtractFeeFromAmount};
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransactionWithTransferAsset(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet, strTxError, coin_control, myAssetOutPoints)) {
        if (!fSubtractFeeFromAmount && Params().IssueAssetBurnAmount() + nFeeRequired > curBalance)
            strTxError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strTxError);
    }

    CValidationState state;
    if (!pwallet->CommitTransaction(wtxNew, reservekey, g_connman.get(), state)) {
        strTxError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strTxError);
    }

    UniValue result(UniValue::VARR);
    result.push_back(wtxNew.GetHash().GetHex());
    return result;
}

//issuefrom(from_address, to_address, qty, units, units=1, reissuable=false)
//Issue an asset with unique name from a specific address -- allows control of which address/private_key is used to issue the asset. Unit as 1 for whole units, or 0.00000001 for satoshi-like units. Qty should be whole number. Reissuable is true/false for whether additional units can be issued by the original issuer.

//issuemore(to_address, asset_name, qty)
//Issue more of a specific asset. This is only allowed by the original issuer of the asset and if the reissuable flag was set to true at the time of original issuance.

//makeuniqueasset(address, asset_name, unique_id)
//Creates a unique asset from a pool of assets with a specific name. Example: If the asset name is SOFTLICENSE, then this could make unique assets like SOFTLICENSE:38293 and SOFTLICENSE:48382 This would be called once per unique asset needed.

//listassets(assets=*, verbose=false, count=MAX, start=0)
//This lists assets that have already been created. It does not distinguish unique assets.

//listuniqueassets(asset)
//This lists the assets that have been made unique, and the address that owns the asset.

//sendasset(to_address, asset, amount)
//This sends assets from one asset holder to another.

//sendassetfrom(from_address, to_address, asset, amount)
//This sends asset from one asset holder to another, but allows specifying which address to send from, so that if a wallet that has multiple addresses holding a given asset, the send can disambiguate the address from which to send.

//getassettransaction(asset, txid)
//This returns details for a specific asset transaction.

//listassettransactions(asset, verbose=false, count=100, start=0)
//This returns a list of transactions for a given asset.

//reward(from_address, asset, amount, except=[])
//Sends RVN to holders of the the specified asset. The Raven is split pro-rata to holders of the asset. Any remainder that cannot be evenly divided to the satoshi (1/100,000,000 RVN) level will be added to the mining fee. ​except​ is a list of addresses to exclude from the distribution - used so that you could exclude treasury shares that do not participate in the reward.

//send_asset(from_address, from_asset, to_asset, amount, except=[])
//Sends an asset to holders of the the specified to_asset. This can be used to send a voting token to holders of an asset. Combined with a messaging protocol explaining the vote, it could act as a distributed voting system.

static const CRPCCommand commands[] =
{ //  category    name                      actor (function)         argNames
  //  ----------- ------------------------  -----------------------  ----------
    { "assets",   "issue",                  &issue,                  {"to_address","asset_name","qty","units","reissuable"} },
    { "assets",   "getaddressbalances",     &getaddressbalances,     {"address", "minconf"} },
    { "assets",   "getallassets",           &getallassets,           {}},
    { "assets",   "getmyassets",            &getmyassets,            {}},
    { "assets",   "getassetaddresses",      &getassetaddresses,      {"asset_name"}},
    { "assets",   "transfer",               &transfer,               {"asset_name, address, amount"}}
};

void RegisterAssetRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
