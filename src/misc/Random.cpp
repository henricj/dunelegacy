#include <misc/Random.h>

#include "misc/SDL2pp.h"
#include "misc/string_util.h"

#if _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4100)
#    pragma warning(disable : 4458)
#endif // _MSC_VER

#include <digestpp/digestpp.hpp>

#if _MSC_VER
#    pragma warning(pop)
#endif // _MSC_VER

#include <array>
#include <atomic>
#include <chrono>
#include <span>
#include <stdexcept>

namespace {
constexpr char master_customization[] = R"(
{
  "pulse" : {
    "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/961833",
    "version" : "Version 2.0",
    "cipherSuite" : 0,
    "period" : 60000,
    "certificateId" : "a2898303d179a120f622e7df9da92bff3f41cecb9f174aedf0b9915728823e7cb180464cb3019f5ce9386c116ed26e53a39df17f0ca88f04e36d8c7470ecc3a4",
    "chainIndex" : 1,
    "pulseIndex" : 961833,
    "timeStamp" : "2020-07-05T08:07:00.000Z",
    "localRandomValue" : "48F45778F003D783BBC4A3707F2415AA6754C9821EB44EF0626CBA58C67BE024FD87E6AE5878269D665E7BB07C61F61ECCF602228DB39BDB379C086F0022C374",
    "external" : {
      "sourceId" : "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
      "statusCode" : 0,
      "value" : "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    },
    "listValues" : [ {
      "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/961832",
      "type" : "previous",
      "value" : "6D7048C8CFCBFF1A881B35B10B111A6A3B08CB0DECF0EB11C687CC890C0738FF0C04EE3228429B47938A2ED03CD7DE76C8848BE662D6F228CF748591A44F62AB"
    }, {
      "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/961826",
      "type" : "hour",
      "value" : "3B68F05D1278FEBF0A66E728200A47C911FFC63D36BBC6583F8638D5AAAA0C7D0C4EA0CE032963ADCB945532B7AAB2E7BA354CF3B57C0AC04FF29FBFB114D469"
    }, {
      "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/961346",
      "type" : "day",
      "value" : "ECAE366443779831BB93C06608F0F33953FEB2EA683FD6CAD2E42103C3D75D17179AD1CB62B89325677BC5BE5F0D5617E9077CC03B559971F7F29CF43736F6DE"
    }, {
      "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/955586",
      "type" : "month",
      "value" : "AD126031D2408CBEF22CD87A694AE667F481B51D8099284F427E0B8731B8C0BAC17A797D39214FD8B2C59BC40CFE0F6B0AF1BC0D3B41B0E6CB556EEF5C113CF8"
    }, {
      "uri" : "https://beacon.nist.gov/beacon/2.0/chain/1/pulse/694662",
      "type" : "year",
      "value" : "CBC9AA97CDD5954218C585C89B061F356EF5F4158622C7CB38FBC317CA69C7ABE9E4379D4738B1076F7671C916C78AD0167A9ADB5A53E0CB20CC7F3D38736857"
    } ],
    "precommitmentValue" : "E8AA0A9C142C9954DED357E4222BC51CDB2E922B79F9377AB089D77E8FB7B497A399034CED6041D9B9C4E5CDC61D5368331D85685CB33A9816AEA950C44A6629",
    "statusCode" : 0,
    "signatureValue" : "684BDA77F3A95A7045E94E3EA31D103778226A4B37B95D74A001F96E89578B07E3F34ED7E9898B1B9EDEF5B5A3126665777768F1B22D4C13810EFDDF3DE014773A82A311021E26A1EC6FF8F2B009A5C5165EE9F3CDCD782810B0851C3D29DB6C50C5D749CC8B0C3528DEEC46C23BD339859CEFE18029474ECC3B547301DC65227F0157D764F0FDE542E38C58EFEE70DF68EFAB9729770E01367535E016BDAFF0DFF30B99E440B9CE4810FB0A5CA8CDBBCD26B00FEFDA047755C00569A4076D9189B714BFDC66C4F15E030DBF953142B53F38060F9C58AEADB9E6E33A8A1AD08F99A402FE5C80DBF8F95C0D2C297F097013EBC8A1B0025124C17989DC6639949A588A478DFC577915C3DB76DF0124AFF950361CAA10E4B198434B373F44CF66C55B889440DEABDFEC51F2E870293435686DDACFF4FC8FE8CDE6C0BF64E770AF5FBF4A8D9C30646BBC5B2592F50216826FB791E65F10BC1C1B4BBFC660A90F2A07B4ADDDCDE3320682F8424A68A3217E7D88C09682C86D237396400D91512B03F9C5A7B4EC978EED596D45C7C50327DE65B596E369ADA281C68BF356FC1688CEA0315EE8CC8A5189C2755EEA039D717E30DEBE2924553FF2FF0BCE1C3B4308B099055AAF4C73BFC127C8A8DE2465BEA2E624A53001912C69B6B77D98E9B0726C6547EF88D461ED5E7E511A7B4EA1B97691C14769E1B7741ED9E1D9A5418C587F67",
    "outputValue" : "A9423F6ACE0182CA81BF568D68BEA5BA5D2E4E30A621834307DBAD0D1B5D481C50A8D570FE3DD80312323FB896A066CF63D2B95669644A296CEEC0113BD55AC9"
  }
}
)";

constexpr char create_customization[] = R"(
{
    "version": "1.0",
    "schema": "http://www.fourmilab.ch/hotbits/json/hotbits.schema.json",
    "status": 200,
    "requestInformation": {
        "serverVersion": "Release 3.9, January 2019",
        "generationTime": "2020-07-05T09:30:52Z",
        "bytesRequested": 512,
        "bytesReturned": 512,
        "quotaRequestsRemaining": 0,
        "quotaBytesRemaining": 0,
        "generatorType": "pseudorandom"
    },
    "data": [
        254, 150, 25, 10, 222, 246, 91, 6, 43, 174, 151, 228, 124, 51, 28, 216,
        212, 109, 208, 252, 116, 101, 253, 14, 19, 21, 187, 181, 57, 254, 179, 140,
        77, 99, 45, 144, 119, 225, 42, 19, 39, 115, 61, 55, 98, 118, 144, 223,
        206, 138, 152, 129, 219, 153, 91, 21, 29, 188, 183, 226, 235, 195, 220, 244,
        57, 237, 16, 51, 200, 126, 111, 49, 182, 181, 122, 206, 22, 244, 23, 100,
        153, 13, 157, 172, 61, 234, 173, 190, 17, 194, 141, 28, 48, 14, 61, 151,
        181, 76, 211, 121, 134, 241, 190, 153, 39, 2, 122, 27, 65, 240, 100, 159,
        158, 83, 222, 89, 229, 172, 100, 199, 202, 188, 48, 206, 239, 253, 80, 120,
        56, 8, 144, 43, 56, 171, 195, 66, 239, 46, 186, 136, 123, 153, 109, 157,
        110, 168, 125, 31, 33, 203, 19, 18, 145, 87, 181, 110, 215, 172, 59, 104,
        183, 235, 101, 98, 125, 150, 155, 154, 166, 40, 8, 9, 86, 211, 96, 183,
        70, 154, 196, 78, 72, 212, 158, 152, 61, 74, 164, 113, 184, 112, 224, 125,
        106, 27, 75, 190, 218, 3, 56, 253, 184, 212, 13, 149, 87, 117, 211, 211,
        152, 211, 11, 190, 9, 213, 13, 63, 197, 186, 22, 198, 88, 19, 42, 218,
        214, 20, 251, 129, 183, 213, 162, 57, 134, 26, 17, 248, 228, 154, 143, 149,
        135, 155, 244, 100, 163, 124, 142, 49, 85, 227, 134, 79, 243, 84, 151, 172,
        145, 130, 166, 121, 246, 46, 204, 176, 221, 154, 31, 8, 62, 64, 135, 152,
        232, 220, 190, 86, 233, 176, 105, 199, 240, 113, 96, 47, 73, 184, 1, 121,
        20, 33, 41, 215, 128, 4, 107, 248, 52, 191, 66, 226, 118, 69, 62, 191,
        15, 33, 103, 215, 11, 133, 167, 42, 20, 190, 141, 106, 202, 189, 182, 19,
        111, 236, 19, 228, 160, 108, 33, 229, 92, 201, 57, 83, 206, 199, 251, 135,
        227, 111, 182, 208, 55, 4, 209, 92, 134, 0, 122, 163, 121, 237, 214, 243,
        65, 223, 220, 39, 220, 71, 35, 42, 74, 129, 234, 179, 34, 92, 210, 191,
        33, 56, 235, 170, 237, 141, 85, 240, 240, 218, 131, 117, 153, 216, 13, 219,
        214, 34, 96, 92, 176, 67, 157, 252, 253, 166, 81, 100, 131, 68, 239, 223,
        171, 145, 226, 189, 98, 223, 210, 33, 29, 85, 249, 143, 179, 54, 85, 17,
        46, 25, 124, 81, 185, 4, 218, 152, 149, 132, 239, 114, 137, 215, 15, 216,
        243, 233, 161, 84, 114, 141, 117, 159, 233, 81, 212, 146, 239, 117, 28, 136,
        48, 127, 161, 18, 115, 218, 128, 21, 90, 98, 5, 111, 82, 104, 73, 127,
        65, 7, 17, 216, 101, 185, 176, 255, 239, 11, 130, 216, 233, 80, 131, 60,
        218, 252, 24, 121, 65, 64, 73, 23, 234, 69, 205, 255, 22, 53, 137, 23,
        234, 176, 228, 87, 231, 190, 251, 6, 225, 107, 188, 138, 96, 38, 103, 101
    ]
})";

// From https://qrng.anu.edu.au/API/jsonI.php?length=8&type=hex16&size=32
constexpr char generate_customization[] = R"(
{"type":"string","length":8,"size":32,"data":["a4f22a5dbc32e69a67f0385289899538041d48283816f60d1e4e5cb63987ac27","af76d9a846d95d369d3ce2ab458ee95e22441f768a155bdf75d9252fb1b1fb6c","a5fffc26f0c9f3ee2787fa42b16763f48253ce8da62eea55ae9001f0534a39e4","712b22a6ea41b3b1f192041bf45df5bd770af20049a815d3ab45f2fd081de216","adffdeb454251227a57d5fcaa7c48c4c5d5d035dfa2b55c9182f926499c42dda","91210fb4b456d420ba6781a91ac195c8dd3d1b4b4bcc053571de1ed8e12ba439","ecc851fe900687249dc5088b51522aa9743b68f8568145fe92d7b6b3cabe3158","6f7acebf7d611023c651eb8ebe60de8d37b1bfd0614f17d6da79f57d2dd722c6"],"success":true}
)";

// https://beacon.inmetro.gov.br/beacon/2.0/pulse/last
constexpr char master_key[] = R"(
{"pulse":{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/386184","version":"Version 2.0","cipherSuite":0,"period":60000,"certificateId":"04c5dc3b40d25294c55f9bc2496fd4fe9340c1308cd073900014e6c214933c7f7737227fc5e4527298b9e95a67ad92e0310b37a77557a10518ced0ce1743e132","chainIndex":1,"pulseIndex":386184,"timeStamp":"2020-07-09T11:53:00.000Z","localRandomValue":"be80a6b78c1cefb52d40c086bffc1f863639527e9f68feefd0b8bb5df1fc91c06ab468aaf93c3812cd4ad578b101f5a41e4e23bd0c18088097f9774766861d6d","external":{"sourceId":"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000","statusCode":0,"value":"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},"listValues":[{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/386183","type":"previous","value":"a4edf2c0b970a7029b6e73f3f4c6d69bbd1cb74a9ea653ed334910d4acd1c79fe2687e3d03f3467d1bb3a1b57b4f122e345e61b31c75a87847ae56d0e5268df2"},{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/386131","type":"hour","value":"243572b76bd52f72338fd87f076c80c9003634e30802662bc485b747d60ebd93ee08d9b70d9a269790c908615655fecd19c99292c44ee1b605cae70ed7f91cf0"},{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/385471","type":"day","value":"9306803b61dcb2d2e18b3722456059728b0318694fb295d9cab6ed296775bb4d57c3f332b366f0bd82fbe082145671db16c6964f0e35b0c445752aec2174aa96"},{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/373951","type":"month","value":"0465eaa0c43ebc92e30860d540751644d624fe5afdc6aac80f699f2ed80c4d14abcf36eab4f5c6b8126fdf32d7122a7ad86b6d9e766c86c46d207bf2ac30d78f"},{"uri":"https://beacon.inmetro.gov.br/beacon/2.0/chain/1/pulse/120164","type":"year","value":"cd0cfb3a32e8faff18aa5b40ce8da4031a96f442daa3240a4c2e2f6d07b75ecee59a5f0dbde06af3a63ca49a55bc8d12d190984998a3f4b0c0cde3c540a5b6c1"}],"precommitmentValue":"90454dace2a748ba09e58409e22a67b62d7962a71f975dd1c7a746302d0bc46e70680d11e71fd7fa2a2d94d59f1e6cc14075795087baed9cfcb2768aae61345d","statusCode":0,"signatureValue":"8b43c0d0111a51826717a218cee5f6a6f5ec1b313f6caf86613e5a29df438ac596440b9fb7d19ea50ee7521d05644ba698f9286e40337282bb91194d20a46d21d2f2a4ae816cf356ee8c495edf5d1c223830f458eb3987d68ae5107a065201d04078468030498c969932d5e28608e9b7dc0b85712c4234e9dc4a49b4f2803aed7423bdab75c86952fa100dc01b472828bcc59bc7a12fa0280ad203cb40da3d46ddfb9009227df1b6b0d2370b72be4ae464f2436fdc4483bbe20a05f62f605224217537b69acd8a1833747408fd5b107ebb1a4d98ddc797f3e5717e3f251b7f70b15fcae8fd4b3ebc3040bf381a69e14f40173f6da21ecf4f02ea62d1cb6c8562a6c8b5c9c687e79f742f461cb486097b7af543b2d424ccbea11ad70c658c6f35c9ad5891534c86501a37a82350472e12fff2e3e65b1937a36550bd21517e0925e0496e05ee0aed1d41af855e2084ebe36d3b1f28c2705c3dcd4daed8f75eef5909302f6e9f6eabd028adce4078bd9ee872c1a2d71b05dd615ae015fedc3cd85ee307c12bbc4742ac6ce07409ffe1f11d9e87b0477566ea72471a3786e3e0d6b54a00fe0d88caade1f90d8a01711c06e608ffa190c392484023343ac371a77a03c8ce8aa775f5154ef8d052a2e48e5d8fbf16caa6740497ba09a7402995bff4730341ec697d00eb35ff73067be1850d0a82d555034f470875918d2b3029e19ccd","outputValue":"f9cc40367dfabeb92e2919f66e1329c1da1cd3850566d14c9d2447a50ed484f3c46cbc445083f33c0d94f12e60f81a126b82db19cebecd97c52f8359588ed563"}}
)";

// https://beacon.clcert.cl/beacon/2.0/pulse/last
constexpr char seed_stir[] = R"(
{"pulse":{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/987497","version":"Version 2.0","cipherSuite":1,"period":60000,"certificateId":"b47eb272812e0105f6900e75f165f1cc6ce25bd5c2758883505673d2fe2c5ac84328e5da21e389d28c5af484fee96c07004528c7ca47501d4f42d972ebb027e8","chainIndex":4,"pulseIndex":987497,"timeStamp":"2020-07-09T11:59:00.000Z","localRandomValue":"d5fedf36f5e2347d110605970a66d27d23e039309cb3338f2d9b001c584c23285dab898b6af03d2ec99fc016b12dc73cb94e0508247ea8bba8a02da39ddce34c","external":[{"sourceId":"ca2c70bc13298c5109ee0cb342d014906e6365249005fd4beee6f01aee44edb531231e98b50bf6810de6cf687882b09320fdd5f6375d1f2debd966fbf8d03efa","value":"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26","statusCode":3},{"sourceId":"564e1971233e098c26d412f2d4e652742355e616fed8ba88fc9750f869aac1c29cb944175c374a7b6769989aa7a4216198ee12f53bf7827850dfe28540587a97","value":"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26","statusCode":3},{"sourceId":"73fb266a903f956a9034d52c2d2793c37fddc32077898f5d871173da1d646fb80bbc21a0522390b75d3bcc88bd78960bdb73be323ad5fc5b3a16089992957d3a","value":"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26","statusCode":3},{"sourceId":"37f558134baa535903c6a88931c8122e334368bf951f2cada569b11774ef9795ef6d2ac961d13ee44a0c837db3817bb9db68ac3bdfb8b19a1308618484a9da8f","value":"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26","statusCode":3}],"listValues":[{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/987493","type":"previous","value":"120f255ac5e0ceab52425ebb550ae29c618d498bebfaf86f1beb9972b9cca6642c9f6911f7081ced03914a9901d4a22b0f5ddc7f7b08b3d32638f00f9cdf3cd9"},{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/987439","type":"hour","value":"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/986779","type":"day","value":"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/975258","type":"month","value":"91976398a544fbc898c1e77b18a815f7f7e933bf82f2fda8fc741dda03f1f354c47d03d8d2cbb45acfce8198734c3ac51c7b64fce2001f653ed8a756b68662ab"},{"uri":"https://beacon.clcert.cl/beacon/2.0/chain/4/pulse/713179","type":"year","value":"f46d2e82aaee3620f77131f9ca1f593497d915e311314a4030e88de163e084b29e5f4b50e4b1d1467dcaca83ed2114c6c3ec4e4dbb4bdb8218157d87876890e8"}],"precommitmentValue":"b6d707d6e8ac084dbb8a8dc7aaf8961bb56abdf83ca0c65493cdd7ca11e34e43ae910f81b11595f856795d7984f0ca8dc2405e71ee749e2d8d292b782ef1aefb","statusCode":101,"signatureValue":"127aac4faffcb25c1872e47da7827dd16de8f09c6ae8f5d6e4230d69116a7f048c95e26fc2cf47d9c91a1a8beed908edb061751f000d472dd1e12612e910201428c8b3e0eb3364f77da49eb9d7e724e61356317ec7ede3687041f3c7736f0919ceb4deb2fad01198ee7b8a3c789e98419f3f9792767e7f8d38354544ce3df23c4e556fd2f2b4bc7e400a4e12b961f3a56d172d894da59d6f5bac0284888b582bd0a32216a172b8bb09ed6e826872f390c5173a427d3bf57ccd52b678656e5909dabaec517a0f9d284fc0a5ada887fb4efc8da57b3ee8823ddde92e27fc80f798d2a0986e1a8df4ee48e24147ae3350f6c3baa5aef1743f321ee7a1cf7e22d21ed3cf4f0605f1135bd67857ea56bda03f0bed6a2b4bb378be6183ca8aaef7bed3aea2ffe58578ad07fc01c8dd1af7c1a2eaa80922e94d3f35b5dd33813d48d6c0d74924833ef967a915c8cf87f849b6ec0e268b6c64987833dafcc6360dd67c6d46f077cbfad65ecf1d4880f538abe4220e3cd5afd529ce652c54ced3fe2dc3a9695572bffb64da74aadefa87aa3071ec899672049c72b657127342e138b6737b47365d4177278c96de97f573b524265e8ede5f833d0f8b0be8dab9eac466300993f5fab5435c384df7e75d7ef40a30625db8218d48dffaa453b4936a0eedbbb5650859fb08169c3f7f0cb8915bc12c884bea83e2ca6dc47bdeba120fa0f04d6c","outputValue":"7d6e0bf2fab34365349d3dcf4c4f2a45840fee9b0343a63a860104ea671156c8ff751c5ba393136fae27b914b62e3776414eb7fee94ca2ea7e0c4dd0f4e26f20","hashedMessage":"6e5fb38b78d45755ae6c8a6a07e734e9ef04bbb7debcc47a76b2bc34c0928fb44350becd09391cf972976f6f2f3ac4d3955a296c1b5a12e45c5e37e697d054f0","witness":"91f3af052ce6ef725e590cec74cf76ff03187ddbf97248096c2942bac253e690ce835800b01779f1eab4cb050bc431536584271d346498dafd008047f058fb0981abf21efb7baaf27b7840d99f3f4dce5434ce3603c1d9e1620ba1dbde3c45e5f1346127672b2e09c75b575750afc8d0005b62bb4b1e63fecedd19f64eb5b93f430b7ddebae23daf486188eac24df93621c6d4d83df51be8a0c4981ab48d69cd37424957aa7831dc6f6f1425718cb50097eb2743689ffef7780c949a6adc59cd661b136c534e87958b3c29c2732b2e5367486c620c1029ff902805d0c7e74614bdca6b1370591c92d95f2fdcd7bc69a8e097d63b5b0651481863b4b489edc21c","iterations":120}}
)";
} // namespace

std::array<uint8_t, Random::state_bytes> Random::getState() const {
    std::array<uint8_t, Random::state_bytes> state;

    get_generator_state(generator_, state);

    sdl2::log_info("Getting state %s", to_hex(state));

    return state;
}

void Random::setState(const std::span<const uint8_t> state) {
    sdl2::log_info("Setting state %s", to_hex(state));

    set_generator_state(generator_, state);
}

void Random::set_generator_state(generator_type& generator, std::span<const uint8_t> state) {
    std::array<generator_type::state_type, generator_type::state_words> words{};

    static_assert(sizeof(generator_type::state_type) == sizeof(uint64_t));

    auto it = state.begin();

    for (auto& word : words) {
        generator_type::state_type n = 0;
        for (auto i = 0u; i < sizeof(decltype(n)); ++i) {
            if (it == state.end())
                break;

            n <<= 8;
            n |= *it++;
        }

        word = n;
    }

    const std::span<generator_type::state_type> state_words =
        it != state.end() ? std::span{words}.subspan(0, it - state.begin()) : words;

    generator.set_state(state_words);
}

void Random::get_generator_state(const generator_type& generator, std::span<uint8_t, state_bytes> state) {
    std::array<generator_type::state_type, generator_type::state_words> words{};

    static_assert(sizeof(generator_type::state_type) == sizeof(uint64_t));

    generator.get_state(words);

    auto it = state.begin();

    for (const auto& word : words) {
        auto n = word;
        for (auto i = 0u; i < sizeof(decltype(n)); ++i) {
            if (it == state.end())
                THROW(std::runtime_error, "Invalid state buffer");

            *it++ = static_cast<uint8_t>(n >> 8 * (sizeof(decltype(n)) - 1));
            n <<= 8;
        }
    }
}

void RandomFactory::setSeed(std::span<const uint8_t> seed) {
    sdl2::log_info("Setting RandomFactory seed to %s", to_hex(seed));

    seed_.clear();
    seed_.reserve(seed.size());

    std::ranges::copy(seed, std::back_inserter(seed_));

    digestpp::kmac256 kmac{8 * key_.size()};

    kmac.set_key(master_key, sizeof master_key);
    kmac.set_customization(&master_customization[0], sizeof master_customization);

    kmac.absorb(&seed_[0], seed_.size());

    kmac.digest(key_.data(), key_.size());

    initialized_ = true;
}

std::vector<uint8_t> RandomFactory::getSeed() const {
    if (!initialized_)
        THROW(std::runtime_error, "RandomFactor::getSeed(): not initialized");

    std::vector<uint8_t> seed;
    seed.reserve(seed_.size());
    std::ranges::copy(seed_, std::back_inserter(seed));

    sdl2::log_info("Getting RandomFactory seed %s", to_hex(seed));

    return seed;
}

std::vector<uint8_t> RandomFactory::createRandomSeed(std::string_view name) {
    std::random_device rd;

    auto entropy_estimate = rd.entropy();

    if (entropy_estimate < 1)
        entropy_estimate = 1;
    else if (entropy_estimate > 30)
        entropy_estimate = 30;

    const auto rd_efficiency = entropy_estimate / (8 * sizeof(decltype(rd())));
    const auto size          = static_cast<int>(std::ceil(seed_size / rd_efficiency)) + 1;

    std::vector<uint32_t> buffer;
    buffer.reserve(size + 5);

    static std::atomic<int> nonce;
    buffer.push_back(++nonce);

    const auto system_now = std::chrono::system_clock::now().time_since_epoch().count();
    buffer.push_back(system_now & ~0U);
    buffer.push_back(system_now >> 32 & ~0U);

    const auto steady_now = std::chrono::steady_clock::now().time_since_epoch().count();
    buffer.push_back(steady_now & ~0U);
    buffer.push_back(steady_now >> 32 & ~0U);

    std::generate_n(std::back_inserter(buffer), size, [&] { return rd(); });

    std::vector<unsigned char> output;
    output.reserve(seed_size);

    digestpp::kmac256 kmac{8 * seed_size};

    kmac.set_key(name.data(), name.size());
    kmac.set_customization(&create_customization[0], sizeof create_customization);

    kmac.absorb(buffer.begin(), buffer.end());

    for (auto i = 0; i < 5; ++i) {
        std::ranges::generate(buffer, [&] { return rd(); });
        kmac.absorb(buffer.begin(), buffer.end());
        kmac.absorb(&seed_stir[0], sizeof seed_stir);
    }

    kmac.digest(std::back_inserter(output));

    sdl2::log_info("Created seed for \"%s\": %s", name, to_hex(output));

    return output;
}

Random RandomFactory::create(std::string_view name) const {
    if (!initialized_)
        THROW(std::runtime_error, "RandomFactor::create(): not initialized");

    static constexpr auto seed_bytes = Random::generator_type::state_words * sizeof(Random::generator_type::state_type);

    digestpp::kmac128 kmac{8 * seed_bytes};

    kmac.set_key(key_.data(), key_.size());
    kmac.set_customization(&generate_customization[0], sizeof generate_customization);

    kmac.absorb(name.data(), name.size());

    std::vector<unsigned char> buffer;
    buffer.reserve(seed_bytes);

    kmac.digest(std::back_inserter(buffer));

    sdl2::log_info("Created state for \"%s\": %s (from %s)", name, to_hex(buffer), to_hex(seed_));

    return Random::create(std::span<const uint8_t, Random::state_bytes>{buffer});
}
