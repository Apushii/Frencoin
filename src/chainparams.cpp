// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2017-2021 The Raven Core developers
// Copyright (c) 2023 The Fren Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "arith_uint256.h"

#include <assert.h>
#include "chainparamsseeds.h"

//TODO: Take these out
extern double algoHashTotal[16];
extern int algoHashHits[16];


static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "NYTimes 05/06/2023 Bank turmoil is paving the way for even bigger shadow banks";
    const CScript genesisOutputScript = CScript() << ParseHex("87be734699ccc6a457d6208847af726ecdaa6f37954dfacfe7bc7d03744f973f6c3ea6706451813734596d01df160b3cb76abcfc4106269a9a48f55cda05b2955b") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

void CChainParams::TurnOffSegwit() {
	consensus.nSegwitEnabled = false;
}

void CChainParams::TurnOffCSV() {
	consensus.nCSVEnabled = false;
}

void CChainParams::TurnOffBIP34() {
	consensus.nBIP34Enabled = false;
}

void CChainParams::TurnOffBIP65() {
	consensus.nBIP65Enabled = false;
}

void CChainParams::TurnOffBIP66() {
	consensus.nBIP66Enabled = false;
}

bool CChainParams::BIP34() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::BIP65() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::BIP66() {
	return consensus.nBIP34Enabled;
}

bool CChainParams::CSVEnabled() const{
	return consensus.nCSVEnabled;
}


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 100000;  // DOGECOIN block schedule 
        consensus.nBIP34Enabled = true;
        consensus.nBIP65Enabled = true; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = true;
        consensus.nCSVEnabled = true;
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.kawpowLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // Estimated starting diff for first 180 kawpow blocks
        consensus.nPowTargetTimespan = 2016 * 60; // 1.4 days
        consensus.nPowTargetSpacing = 1 * 30;
		consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1008; // Approx 50% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1713632231; // Saturday, April 20, 2024 
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideRuleChangeActivationThreshold = 1008;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;  //Assets (RIP2)
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 1713632231; // Saturday, April 20, 2024
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideRuleChangeActivationThreshold = 1008;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].bit = 7;  // Assets (RIP5)
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nStartTime = 1713632231; // Saturday, April 20, 2024
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideRuleChangeActivationThreshold = 1008; // Approx 50% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nStartTime = 1713632231; // Saturday, April 20, 2024
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideRuleChangeActivationThreshold = 1008; // Approx 50% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nStartTime = 1713632231; // Saturday, April 20, 2024
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideRuleChangeActivationThreshold = 1008; // Approx 50% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].bit = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nStartTime = 1713632231; // Saturday, April 20, 2024
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nTimeout = 1745168231; // Sunday, April 20, 2025 
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideRuleChangeActivationThreshold = 1008; // Approx 50% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideMinerConfirmationWindow = 2016;


        // The best chain should have at least this much work
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000006624f6ad0a2d554"); // Block 1220162

        // By default assume that the signatures in ancestors of this block are valid. 
        consensus.defaultAssumeValid = uint256S("0x00000000013a474999cfa60ededb13b8c1f20d1f1031671af868cac691caa8df"); // Block 1220162

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x41; // A
        pchMessageStart[1] = 0x50; // P
        pchMessageStart[2] = 0x55; // U
        pchMessageStart[3] = 0x53; // S
        nDefaultPort = 4207;
        nPruneAfterHeight = 100000;

        uint32_t nGenesisTime = 1683401142;  // Sat, 06 May 2023 


        genesis = CreateGenesisBlock(nGenesisTime, 11859329, 0x1e00ffff, 4, 2000000 * COIN);

        consensus.hashGenesisBlock = genesis.GetX16RHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00000022bcd010da6b8d3c2c0e80229ea1f5c0bb4960678d3c26bab2c9b23b9a"));
        assert(genesis.hashMerkleRoot == uint256S("826ffa0d97ee1c9b8ce96f862673b89be0a3e3a4d0be3b42798e776e51b77eb5"));

        vSeeds.emplace_back("seed.frencoin.net", false);
        vSeeds.emplace_back("fren.ignorelist.com", false);
        vSeeds.emplace_back("frenseed.ignorelist.com ", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,35); // F
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,95); // f
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128); // t
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        // Fren BIP44 cointype in mainnet is '42066'
        nExtCoinType = 42066;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fMiningRequiresPeers = true;

        checkpointData = (CCheckpointData) {
            {
                { 0, uint256S("0x00000022bcd010da6b8d3c2c0e80229ea1f5c0bb4960678d3c26bab2c9b23b9a")},
                { 180, uint256S("0x00c1d3ead56cf13f9bcfdf187e9fca725fa837fc5e32a7174c8329d08399a4e9")},
                { 4391, uint256S("0x00000000e7490492559fb07a78e5a7181739be4ad262db016b197f04a0e06ed6")},
                { 8344, uint256S("0x00000000e4f9c29ef961026e681750fd310347e48dfa338c406fa78b553b1ff6")},
                { 19521, uint256S("0x00000001952b6f1d0d1fec21bda9173de9a705852d1da8688cfd8df622d929a8")},
                { 37211, uint256S("0x000000001c9bea4630d650d120ed3101f973a0795fb8506eed0f4fc1e6b891ae")},
                { 83249, uint256S("0x0000000007aa01e5f0d42771ceb9826ec210bd3d90c3d9488808ed448e80a0a5")},
                { 145745, uint256S("0x000000000210c2769f7c0c4ff9f068ccfe619ce60751f91c41e8bc1763c1ebe1")},
                { 279479, uint256S("0x0000000001117ac273717953ea216bc8ec05117706a669b3b1ca6dd0ef531dca")},
                { 579685, uint256S("0x0000000000031fa705b341270aeca4ba3d33c0aa49a8d23a759cded49e8baab2")},
                { 1220162, uint256S("0x00000000013a474999cfa60ededb13b8c1f20d1f1031671af868cac691caa8df")}
            }
        };


        chainTxData = ChainTxData{
            // Update as we know more about the contents of the Fren chain
            // Stats as of 
            1720394570, // * UNIX timestamp of last known number of transactions
            1709258,    // * total number of transactions between genesis and that timestamp
                        //   getchaintxstats(the tx=... number in the SetBestChain debug.log lines)
            0.0378       // * estimated number of transactions per second after that timestamp
        };

        /** FRENS Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 500 * COIN;
        nReissueAssetBurnAmount = 100 * COIN;
        nIssueSubAssetBurnAmount = 100 * COIN;
        nIssueUniqueAssetBurnAmount = 5 * COIN;
        nIssueMsgChannelAssetBurnAmount = 100 * COIN;
        nIssueQualifierAssetBurnAmount = 1000 * COIN;
        nIssueSubQualifierAssetBurnAmount = 100 * COIN;
        nIssueRestrictedAssetBurnAmount = 1500 * COIN;
        nAddNullQualifierTagBurnAmount = .1 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strReissueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueUniqueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueMsgChannelAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueRestrictedAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strAddNullQualifierTagBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

            //Global Burn Address
        strGlobalBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

        // DGW Activation
        nDGWActivationBlock = 1;

        nMaxReorganizationDepth = 60; // 60 at 1 minute block timespan is +/- 30 minutes.
        nMinReorganizationPeers = 4;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours

        nAssetActivationHeight = 1051200; // Asset activated block height
        nMessagingActivationBlock = 1051200; // Messaging activated block height
        nRestrictedActivationBlock = 1051200; // Restricted activated block height

        nKAAAWWWPOWActivationTime = 1683401242; // UTC: Saturday May 06 2023 
        nKAWPOWActivationTime = nKAAAWWWPOWActivationTime;
        /** FRENS End **/
    }
};

/**
 * Testnet (v7)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 100000;  // Dogecoin Block Schedule
        consensus.nBIP34Enabled = true;
        consensus.nBIP65Enabled = true; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = true;
        consensus.nCSVEnabled = true;

        consensus.powLimit = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.kawpowLimit = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2016 * 60; // 1.4 days
        consensus.nPowTargetSpacing = 1 * 30;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1310; // Approx 65% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideRuleChangeActivationThreshold = 1310;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;  //Assets (RIP2)
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideRuleChangeActivationThreshold = 1310;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].bit = 7;  // Assets (RIP5)
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideRuleChangeActivationThreshold = 1310; // Approx 85% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideRuleChangeActivationThreshold = 1310; // Approx 85% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideRuleChangeActivationThreshold = 1411; // Approx 70% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].bit = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nStartTime = 1683392611; // UTC: Saturday May 06 2023
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nTimeout = 3133702631; // Saturday, April 20, 2069
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideRuleChangeActivationThreshold = 1411; // Approx 70% of 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideMinerConfirmationWindow = 2016;

        // The best chain should have at least this much work.
        // consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000168050db560b4");

        // By default assume that the signatures in ancestors of this block are valid.
        // consensus.defaultAssumeValid = uint256S("0x0000007f6a971cc1ca519e31ac6ce0a065edf8c69f6fb214e75c6a6e0a1004b0");


        pchMessageStart[0] = 0x48; // H
        pchMessageStart[1] = 0x45; // E
        pchMessageStart[2] = 0x4C; // L
        pchMessageStart[3] = 0x50; // P
        nDefaultPort = 14207;
        nPruneAfterHeight = 1000;

        uint32_t nGenesisTime = 1683393259;  // Sat, 06 May 2023 
        // This is used inorder to mine the genesis block. Once found, we can use the nonce and block hash found to create a valid genesis block
        // /////////////////////////////////////////////////////////////////


        // arith_uint256 test;
        // bool fNegative;
        // bool fOverflow;
        // test.SetCompact(0x1e00ffff, &fNegative, &fOverflow);
        // std::cout << "Test threshold: " << test.GetHex() << "\n\n";

        // int genesisNonce = 0;
        // uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        // uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        // for (int i=0;i<40000000;i++) {
        //     genesis = CreateGenesisBlock(nGenesisTime, i, 0x1e00ffff, 2, 2000000 * COIN);
        //     //genesis.hashPrevBlock = TempHashHolding;
        //     // Depending on when the timestamp is on the genesis block. You will need to use GetX16RHash or GetX16RV2Hash. Replace GetHash() with these below
        //     consensus.hashGenesisBlock = genesis.GetX16RV2Hash();

        //     arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
        //     if (UintToArith256(consensus.hashGenesisBlock) < BestBlockHashArith) {
        //         BestBlockHash = consensus.hashGenesisBlock;
        //         std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
        //         std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
        //     }

        //     TempHashHolding = consensus.hashGenesisBlock;

        //     if (BestBlockHashArith < test) {
        //         genesisNonce = i - 1;
        //         break;
        //     }
        //     //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
        // }
        // std::cout << "\n";
        // std::cout << "\n";
        // std::cout << "\n";

        // std::cout << "Test hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
        // std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
        // std::cout << "Genesis Merkle " << genesis.hashMerkleRoot.GetHex() << std::endl;

        // std::cout << "\n";
        // std::cout << "\n";
        // int totalHits = 0;
        // double totalTime = 0.0;

        // for(int x = 0; x < 16; x++) {
        //     totalHits += algoHashHits[x];
        //     totalTime += algoHashTotal[x];
        //     std::cout << "hash algo " << x << " hits " << algoHashHits[x] << " total " << algoHashTotal[x] << " avg " << algoHashTotal[x]/algoHashHits[x] << std::endl;
        // }

        // std::cout << "Totals: hash algo " <<  " hits " << totalHits << " total " << totalTime << " avg " << totalTime/totalHits << std::endl;

        // genesis.hashPrevBlock = TempHashHolding;

        // return;

        // /////////////////////////////////////////////////////////////////

        genesis = CreateGenesisBlock(nGenesisTime, 19782265, 0x1e00ffff, 2, 2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetX16RHash();

        //Test MerkleRoot and GenesisBlock
        assert(consensus.hashGenesisBlock == uint256S("0x000000d3e5277d279bdbf8c752ff8c1b3a6ae1ac30490a6d1b8f4cd8e7256548"));
        assert(genesis.hashMerkleRoot == uint256S("826ffa0d97ee1c9b8ce96f862673b89be0a3e3a4d0be3b42798e776e51b77eb5"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.emplace_back("test-frencoin.mooo.com", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,100);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Fren BIP44 cointype in testnet
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fMiningRequiresPeers = true;

        checkpointData = (CCheckpointData) {
            {
                    { 0, uint256S("0x000000d3e5277d279bdbf8c752ff8c1b3a6ae1ac30490a6d1b8f4cd8e7256548")},
            }
        };

        chainTxData = ChainTxData{
            // Update as we know more about the contents of the Fren chain
            // Stats as of 00000023b66f46d74890287a7b1157dd780c7c5fdda2b561eb96684d2b39d62e window size 43200
            nGenesisTime, // * UNIX timestamp of last known number of transactions
            0,     // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.02        // * estimated number of transactions per second after that timestamp
        };

        /** FRENS Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 500 * COIN;
        nReissueAssetBurnAmount = 100 * COIN;
        nIssueSubAssetBurnAmount = 100 * COIN;
        nIssueUniqueAssetBurnAmount = 5 * COIN;
        nIssueMsgChannelAssetBurnAmount = 100 * COIN;
        nIssueQualifierAssetBurnAmount = 1000 * COIN;
        nIssueSubQualifierAssetBurnAmount = 100 * COIN;
        nIssueRestrictedAssetBurnAmount = 1500 * COIN;
        nAddNullQualifierTagBurnAmount = .1 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strReissueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueUniqueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueMsgChannelAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueRestrictedAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strAddNullQualifierTagBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

            //Global Burn Address
        strGlobalBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

        // DGW Activation
        nDGWActivationBlock = 1;

        nMaxReorganizationDepth = 60; // 60 at 1 minute block timespan is +/- 60 minutes.
        nMinReorganizationPeers = 4;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours

        nAssetActivationHeight = 1; // Asset activated block height
        nMessagingActivationBlock = 1; // Messaging activated block height
        nRestrictedActivationBlock = 1; // Restricted activated block height

        nKAAAWWWPOWActivationTime = 1683393385; //Wed Mar 25 2020 18:00:00 UTC
        nKAWPOWActivationTime = nKAAAWWWPOWActivationTime;
        /** FRENS End **/
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nBIP34Enabled = true;
        consensus.nBIP65Enabled = true; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.nBIP66Enabled = true;
        consensus.nSegwitEnabled = true;
        consensus.nCSVEnabled = true;
        consensus.nSubsidyHalvingInterval = 100;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.kawpowLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2016 * 60; // 1.4 days
        consensus.nPowTargetSpacing = 1 * 30;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideRuleChangeActivationThreshold = 108;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nOverrideMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideRuleChangeActivationThreshold = 108;
        consensus.vDeployments[Consensus::DEPLOYMENT_ASSETS].nOverrideMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].bit = 7;  // Assets (RIP5)
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nStartTime = 0; // GMT: Sun Mar 3, 2019 5:00:00 PM
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nTimeout = 999999999999ULL; // UTC: Wed Dec 25 2019 07:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideRuleChangeActivationThreshold = 108;
        consensus.vDeployments[Consensus::DEPLOYMENT_MSG_REST_ASSETS].nOverrideMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideRuleChangeActivationThreshold = 208;
        consensus.vDeployments[Consensus::DEPLOYMENT_TRANSFER_SCRIPT_SIZE].nOverrideMinerConfirmationWindow = 288;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideRuleChangeActivationThreshold = 108;
        consensus.vDeployments[Consensus::DEPLOYMENT_ENFORCE_VALUE].nOverrideMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].bit = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideRuleChangeActivationThreshold = 400;
        consensus.vDeployments[Consensus::DEPLOYMENT_COINBASE_ASSETS].nOverrideMinerConfirmationWindow = 500;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0x43; // C
        pchMessageStart[1] = 0x4F; // O
        pchMessageStart[2] = 0x49; // I
        pchMessageStart[3] = 0x4E; // N
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

// This is used inorder to mine the genesis block. Once found, we can use the nonce and block hash found to create a valid genesis block
    //    /////////////////////////////////////////////////////////////////


    //    arith_uint256 test;
    //    bool fNegative;
    //    bool fOverflow;
    //    test.SetCompact(0x207fffff, &fNegative, &fOverflow);
    //    std::cout << "Test threshold: " << test.GetHex() << "\n\n";

    //    int genesisNonce = 0;
    //    uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
    //    uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    //    for (int i=0;i<40000000;i++) {
    //        genesis = CreateGenesisBlock(1683393655, i, 0x207fffff, 2, 2000000 * COIN);
    //        //genesis.hashPrevBlock = TempHashHolding;
    //        consensus.hashGenesisBlock = genesis.GetX16RV2Hash();

    //        arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
    //        if (UintToArith256(consensus.hashGenesisBlock) < BestBlockHashArith) {
    //            BestBlockHash = consensus.hashGenesisBlock;
    //            std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
    //            std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
    //        }

    //        TempHashHolding = consensus.hashGenesisBlock;

    //        if (BestBlockHashArith < test) {
    //            genesisNonce = i - 1;
    //            break;
    //        }
    //        //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
    //    }
    //    std::cout << "\n";
    //    std::cout << "\n";
    //    std::cout << "\n";

    //    std::cout << "RegTest hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
    //    std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
    //    std::cout << "Genesis Merkle " << genesis.hashMerkleRoot.GetHex() << std::endl;

    //    std::cout << "\n";
    //    std::cout << "\n";
    //    int totalHits = 0;
    //    double totalTime = 0.0;

    //    for(int x = 0; x < 16; x++) {
    //        totalHits += algoHashHits[x];
    //        totalTime += algoHashTotal[x];
    //        std::cout << "hash algo " << x << " hits " << algoHashHits[x] << " total " << algoHashTotal[x] << " avg " << algoHashTotal[x]/algoHashHits[x] << std::endl;
    //    }

    //    std::cout << "Totals: hash algo " <<  " hits " << totalHits << " total " << totalTime << " avg " << totalTime/totalHits << std::endl;

    //    genesis.hashPrevBlock = TempHashHolding;

    //    return;

    //    /////////////////////////////////////////////////////////////////


        genesis = CreateGenesisBlock(1683393655, 2, 0x207fffff, 4,2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetX16RHash();

        assert(consensus.hashGenesisBlock == uint256S("0x7edc123ca862994d9ccb1ed919db9ba12fd4e4540ae881ab8e0001b1a9199ee4 "));
        assert(genesis.hashMerkleRoot == uint256S("826ffa0d97ee1c9b8ce96f862673b89be0a3e3a4d0be3b42798e776e51b77eb5"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = (CCheckpointData) {
            {
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Fren BIP44 cointype in regtest
        nExtCoinType = 1;

        /** FRENS Start **/
        // Burn Amounts
        nIssueAssetBurnAmount = 500 * COIN;
        nReissueAssetBurnAmount = 100 * COIN;
        nIssueSubAssetBurnAmount = 100 * COIN;
        nIssueUniqueAssetBurnAmount = 5 * COIN;
        nIssueMsgChannelAssetBurnAmount = 100 * COIN;
        nIssueQualifierAssetBurnAmount = 1000 * COIN;
        nIssueSubQualifierAssetBurnAmount = 100 * COIN;
        nIssueRestrictedAssetBurnAmount = 1500 * COIN;
        nAddNullQualifierTagBurnAmount = .1 * COIN;

        // Burn Addresses
        strIssueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strReissueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueUniqueAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueMsgChannelAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueSubQualifierAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strIssueRestrictedAssetBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";
        strAddNullQualifierTagBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

        // Global Burn Address
        strGlobalBurnAddress = "FNwgpD57TYBbqzSr86qXttRgZxYYumcfyK";

        // DGW Activation
        nDGWActivationBlock = 200;

        nMaxReorganizationDepth = 60; // 60 at 1 minute block timespan is +/- 60 minutes.
        nMinReorganizationPeers = 4;
        nMinReorganizationAge = 60 * 60 * 12; // 12 hours

        nAssetActivationHeight = 0; // Asset activated block height
        nMessagingActivationBlock = 0; // Messaging activated block height
        nRestrictedActivationBlock = 0; // Restricted activated block height

        // TODO, we need to figure out what to do with this for regtest. This effects the unit tests
        // For now we can use a timestamp very far away
        // If you are looking to test the kawpow hashing function in regtest. You will need to change this number
        nKAAAWWWPOWActivationTime = 3582830167;
        nKAWPOWActivationTime = nKAAAWWWPOWActivationTime;
        /** FRENS End **/
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &GetParams() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network, bool fForceBlockNetwork)
{
    SelectBaseParams(network);
    if (fForceBlockNetwork) {
        bNetwork.SetNetwork(network);
    }
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}

void TurnOffSegwit(){
	globalChainParams->TurnOffSegwit();
}

void TurnOffCSV() {
	globalChainParams->TurnOffCSV();
}

void TurnOffBIP34() {
	globalChainParams->TurnOffBIP34();
}

void TurnOffBIP65() {
	globalChainParams->TurnOffBIP65();
}

void TurnOffBIP66() {
	globalChainParams->TurnOffBIP66();
}
