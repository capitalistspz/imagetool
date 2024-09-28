#include <vector>
#include <algorithm>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/sha.h>
#include <crypto++/rijndael.h>
#include <crypto++/cryptlib.h>
#include <crypto++/filters.h>
//#include <print>
#include "File.hpp"
#include "bstype.h"
#include "util.hpp"

enum class SignatureType
{
   ECDSA = 0x01,
   RSA = 0x02
};

enum class TargetDevice
{
   NAND = 0x21,
   SD = 0x22
};

enum class ConsoleType
{
   Dev = 1,
   Prod = 2
};


int main(int argc, const char** argv)
{
   if (argc < 3)
   {
      //std::println(stderr, "Usage:\n\timagetool /path/to/otp /path/to/fw.img");
      return EXIT_FAILURE;
   }

   auto otp = RBFile(argv[1]);
   otp.Seek(0x090);
   const auto ancastKey = otp.Read<std::array<u8, 0x10>>();
   using namespace CryptoPP;

   SHA1 hash;
   hash.Update(ancastKey.data(), ancastKey.size());
   std::vector<u8> digest(hash.DigestSize());
   hash.Final(digest.data());

   const auto digestString = to_hex_string(digest);
   constexpr auto expectedDigest = "d8b4970a7ed12e1002a0c4bf89bee171740d268b";
   exit_assert(digestString == expectedDigest, "Ancast Key SHA1 digest was {}, expected {}", digestString, expectedDigest);

   auto fwFile = RBFile(argv[2]);

   constexpr static u32 magicNum = 0xEFA282D9;
   exit_assert(fwFile.Read<u32be>() == magicNum, "Expected magic number to be 0xEFA282D9");
   exit_assert(fwFile.Read<u32>() == 0, "Value should be zero");
   const auto signatureOffset = fwFile.Read<u32be>();
   exit_assert(signatureOffset == 0x20, "Expected signature offset to be 0x20");

   exit_assert(is_all_zero(fwFile.Read<std::array<u8,20>>()), "Value should be zero");

   const SignatureType sigType = fwFile.Read<betype<SignatureType>>();
   exit_assert(sigType == SignatureType::RSA, "Expected signature type to be RSA.");

   const auto imageSignature = fwFile.Read<std::array<u8, 0x100>>();

   const auto padding = fwFile.Read<std::array<u8, 0x7c>>();
   exit_assert(is_all_zero(padding), "Expected 0x7c-byte padding at 0x124 to be zeroed.");

   exit_assert(fwFile.Read<u32>() == 0, "Values should be zero");

   const TargetDevice targetDevice = fwFile.Read<betype<TargetDevice>>();

   const ConsoleType consoleType = fwFile.Read<betype<ConsoleType>>();

   const u32 imageBodySize = fwFile.Read<u32be>();

   const auto imageBodyHash = fwFile.Read<std::array<u8, 0x14>>();

   const u32 version = fwFile.Read<u32be>();

   const auto padding2 = fwFile.Read<std::array<u8, 0x38>>();
   exit_assert(is_all_zero(padding2), "Expected 0x38-byte padding at 0x1c8 to be zeroed");

   auto startPos = fwFile.Tell();
   fwFile.Seek(0, SeekFrom::End);
   auto endPos = fwFile.Tell();
   fwFile.Seek(startPos);
   std::vector<u8> encryptedData(endPos - startPos);
   fwFile.ReadToRange(encryptedData);


   std::array<u8, AES::BLOCKSIZE> initVector = {};
   CBC_Mode<AES>::Decryption decrypt(ancastKey.data(),AES::DEFAULT_KEYLENGTH,initVector.data());

   std::vector<byte> decryptedData;
   VectorSource s(encryptedData, true,
      new StreamTransformationFilter(decrypt,
         new VectorSink(decryptedData)));

   return EXIT_SUCCESS;
}
