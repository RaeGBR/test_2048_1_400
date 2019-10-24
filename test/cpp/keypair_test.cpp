#include "gtest/gtest.h"

#include "ECCurve.hpp"
#include "KeyPair.hpp"
#include "lib/ec/ECKeyPairImpl.hpp"
#include "Utils.hpp"

using namespace std;
using namespace cryptoplus;

namespace
{

TEST(KeyPair, GEN_KEY)
{
  // Normal Seed
  auto curve = ECCurve::SECP256K1();
  vector<uint8_t> seed = {0x48, 0x41, 0x48, 0x41, 0x48, 0x41};

  auto pair1 = KeyPair::createWithSeed(curve, seed);
  auto pair2 = KeyPair::createWithSeed(curve, seed);

  EXPECT_EQ(pair1->getPrivateKey(), pair2->getPrivateKey());
  EXPECT_EQ(pair1->getPublicKey(), pair2->getPublicKey());

  // Long Seed
  vector<uint8_t> seed2 = {0x49, 0x20, 0x41, 0x4d, 0x20, 0x41, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x56, 0x45, 0x52, 0x59, 0x20, 0x4c, 0x4f, 0x4e, 0x47, 0x20, 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45};

  auto pair3 = KeyPair::createWithSeed(curve, seed2);
  auto pair4 = KeyPair::createWithSeed(curve, seed2);

  EXPECT_EQ(pair3->getPrivateKey(), pair4->getPrivateKey());
  EXPECT_EQ(pair3->getPublicKey(), pair4->getPublicKey());

  // Test the Seed is not 32-bytes (256-bits) fixed length
  string origin = "I AM A VERY VERY VERY LONG SENTE";
  string spoof = "I AM A VERY VERY VERY LONG SENTEN";

  auto pair5 = KeyPair::createWithSeed(curve, vector<uint8_t>(origin.begin(), origin.end()));
  auto pair6 = KeyPair::createWithSeed(curve, vector<uint8_t>(spoof.begin(), spoof.end()));
  EXPECT_NE(pair5->getPrivateKey(), pair4->getPrivateKey());
  EXPECT_NE(pair5->getPrivateKey(), pair6->getPrivateKey());
}

TEST(KeyPair, GEN_KEY_Java_Ver)
{
  auto curve = ECCurve::SECP256K1();
  auto k1 = KeyPair::create(curve);
  auto k2 = KeyPair::create(curve);
  string seed1 = "123";
  string seed2 = "I'm a very long string, that very long very long very long very long very long very long very long very long very long";
  auto k3 = KeyPair::createWithSeed(curve, vector<uint8_t>(seed1.begin(), seed1.end()));
  auto k4 = KeyPair::createWithSeed(curve, vector<uint8_t>(seed1.begin(), seed1.end()));
  auto k5 = KeyPair::createWithSeed(curve, vector<uint8_t>(seed2.begin(), seed2.end()));
  auto k6 = KeyPair::createWithSeed(curve, vector<uint8_t>(seed2.begin(), seed2.end()));

  ASSERT_FALSE(k1->eq(k2));
  ASSERT_FALSE(k2->eq(k3));
  ASSERT_TRUE(k3->eq(k4));
  ASSERT_FALSE(k4->eq(k5));
  ASSERT_TRUE(k5->eq(k6));
}

TEST(KeyPair, Predictable)
{
  // Normal Seed
  auto curve = ECCurve::SECP256K1();
  string seed = "Hello, my name is Alex";

  auto pair = KeyPair::createWithSeed(curve, vector<uint8_t>(seed.begin(), seed.end()));
  vector<uint8_t> privateKey = pair->getPrivateKey();
  vector<uint8_t> publicKey = pair->getPublicKey();
  auto validPair = make_shared<KeyPairImpl>(
      curve,
      privateKey,
      publicKey);

  EXPECT_EQ(pair->getPrivateKey(), validPair->getPrivateKey());
  EXPECT_EQ(pair->getPublicKey(), validPair->getPublicKey());
}

TEST(KeyPair, KeyPairMathTest)
{
  auto curve = ECCurve::SECP256K1();
  auto pair = KeyPair::create(curve);
  auto x = pair->getPrivateElement();
  auto y = pair->getPublicElement();
  auto gx = curve->getG()->mul(curve, x);

  ASSERT_TRUE(y->eq(gx));
}

TEST(KeyPair, PublicKey_From_PrivateKey)
{
  auto curve = ECCurve::SECP256K1();
  string seed = "Hello, my name is Alex :)";

  auto pair = KeyPair::createWithSeed(curve, vector<uint8_t>(seed.begin(), seed.end()));
  auto validPair = KeyPair::createWithPrivateKey(curve, pair->getPrivateKey());

  EXPECT_EQ(pair->getPrivateKey(), validPair->getPrivateKey());
  EXPECT_EQ(pair->getPublicKey(), validPair->getPublicKey());
}
} // namespace