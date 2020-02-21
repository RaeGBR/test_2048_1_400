#include "./App.hpp"

void polyu::run(size_t byteLength, size_t msgCount, size_t rangeProofCount, size_t slotSize, size_t msgPerBatch, ofstream &fs)
{
  auto crypto = PaillierEncryption::generate(byteLength);
  polyu::run(crypto, msgCount, rangeProofCount, slotSize, msgPerBatch, fs);
}

void polyu::run(const shared_ptr<PaillierEncryption> &crypto, size_t msgCount, size_t rangeProofCount, size_t slotSize, size_t msgPerBatch, ofstream &fs)
{
  auto GP_Q = crypto->getGroupQ();
  auto GP_P = crypto->getGroupP();
  auto GP_G = crypto->getGroupG();
  auto pk = crypto->getPublicKey();
  auto sk = crypto->getPrivateKey();
  auto byteLength = pk->toBinary().size();

  double encryptTime = 0;
  double circuitTime = 0;
  double valueTime = 0;
  double commitTime = 0;
  double proveTime = 0;
  double verifyTime = 0;

  auto encCir = make_shared<CEnc>(crypto);
  encCir->wireUp();
  auto encCirN = encCir->gateCount;
  auto encCirQ = encCir->linearCount;

  /*
   * Prover
   */
  auto decryptor = make_shared<PaillierEncryption>(pk, sk, GP_Q, GP_P, GP_G);
  auto proverCir = make_shared<CBatchEnc>(decryptor, msgCount, rangeProofCount, slotSize, msgPerBatch);

  // P: prover prepare structured message
  vector<shared_ptr<Integer>> msg;
  for (size_t i = 0; i < msgCount; i++)
  {
    auto maxInt = (int)pow(2, proverCir->slotsPerMsg);
    auto max = make_shared<IntegerImpl>(maxInt);
    string mStr(proverCir->msgSize * 2, '0');
    auto iStr = make_shared<IntegerImpl>(i)->mod(max)->toBinaryString();

    for (int j = 0; j < iStr.size() && j < proverCir->slotsPerMsg; j++)
    {
      int jIdx = iStr.size() - j - 1;
      char v = iStr[jIdx];
      size_t mIdx = (proverCir->slotsPerMsg - j) * slotSize * 2 - 1;
      mStr[mIdx] = v;
    }
    auto m = make_shared<IntegerImpl>(mStr.c_str(), 16);
    msg.push_back(m);
  }

  // P: prover batch encrypt message
  Timer::start("P.encrypt");
  proverCir->encrypt(msg);
  encryptTime += Timer::end("P.encrypt");

  // P: non-interactive mode, prover calculate challenge value by itself
  Timer::start("P.Ljir");
  auto ljir1 = proverCir->calculateLjir();
  circuitTime += Timer::end("P.Ljir");

  // P: prover compute Lj for range proof
  Timer::start("P.Lj");
  auto Lj = proverCir->calculateLj(ljir1);
  circuitTime += Timer::end("P.Lj");

  // P: create circuit constrains
  Timer::start("P.wireUp");
  proverCir->wireUp(ljir1, Lj);
  circuitTime += Timer::end("P.wireUp");

  // P: assign circuit values
  Timer::start("P.run");
  proverCir->run(ljir1, Lj);
  valueTime += Timer::end("P.run");

  // P: setup ZKP protocol for the circuit
  Timer::start("P.circuit");
  auto mnCfg1 = CircuitZKPVerifier::calcMN(proverCir->gateCount);
  auto m1 = mnCfg1[0];
  auto n1 = mnCfg1[1];
  proverCir->group(n1, m1);
  proverCir->trim();

  auto proverZkp = make_shared<CircuitZKPVerifier>(
      GP_Q, GP_P, GP_G,
      proverCir->Wqa, proverCir->Wqb, proverCir->Wqc, proverCir->Kq,
      m1, n1);
  auto prover = make_shared<CircuitZKPProver>(proverZkp, proverCir->A, proverCir->B, proverCir->C);
  circuitTime += Timer::end("P.circuit");

  // P: prover commit the circuit arguments
  Timer::start("P.commit");
  vector<shared_ptr<Integer>> commits;
  prover->commit(commits);
  commitTime += Timer::end("P.commit");

  // P: prover calculate challenge value Y (non-interactive mode)
  Timer::start("P.y");
  prover->zkp->setCommits(commits);
  auto y1 = prover->zkp->calculateY();
  proveTime += Timer::end("P.y");

  // P: prover perform polyCommit
  Timer::start("P.polyCommit");
  vector<shared_ptr<Integer>> pc;
  prover->polyCommit(y1, pc);
  proveTime += Timer::end("P.polyCommit");

  // P: prover calculate challenge value X (non-interactive mode)
  Timer::start("P.x");
  prover->zkp->setPolyCommits(pc);
  auto x1 = prover->zkp->calculateX();
  proveTime += Timer::end("P.x");

  // P: prover calculate the ZKP
  Timer::start("P.prove");
  vector<shared_ptr<Integer>> proofs;
  prover->prove(y1, x1, proofs);
  proveTime += Timer::end("P.prove");

  /*
   * Verifier
   */
  auto encryptor = make_shared<PaillierEncryption>(pk, GP_Q, GP_P, GP_G);
  auto verifierCir = make_shared<CBatchEnc>(encryptor, msgCount, rangeProofCount, slotSize, msgPerBatch);

  // P->V: Cm, Cm', CR'j, Lj (circuit wire up)
  //       commits           (commits of A, B, C, D)
  //       pc                (commits of polynomial t(X))
  //       proofs            (pe, r, rr)
  // V: verifier receive cipher and calculate challenge (lj)
  auto Cm = proverCir->Cm;
  auto Cm_ = proverCir->Cm_;
  auto CRj = proverCir->CRj;

  Timer::start("V.Ljir");
  verifierCir->setCipher(Cm, Cm_, CRj);
  auto ljir2 = verifierCir->calculateLjir();
  circuitTime += Timer::end("V.Ljir");

  // P: create circuit constrains
  Timer::start("V.wireUp");
  verifierCir->wireUp(ljir2, Lj);
  circuitTime += Timer::end("V.wireUp");

  // P: setup ZKP protocol for the circuit
  Timer::start("V.circuit");
  auto mnCfg2 = CircuitZKPVerifier::calcMN(verifierCir->gateCount);
  auto m2 = mnCfg2[0];
  auto n2 = mnCfg2[1];
  verifierCir->group(n2, m2);
  verifierCir->trim();

  auto verifier = make_shared<CircuitZKPVerifier>(
      GP_Q, GP_P, GP_G,
      verifierCir->Wqa, verifierCir->Wqb, verifierCir->Wqc, verifierCir->Kq,
      m2, n2);
  circuitTime += Timer::end("V.circuit");

  // V: verifier calculate challenge value Y
  Timer::start("V.y");
  verifier->setCommits(commits);
  auto y = verifier->calculateY();
  verifyTime += Timer::end("V.y");

  // V: verifier calculate challenge value X
  Timer::start("V.x");
  verifier->setPolyCommits(pc);
  auto x = verifier->calculateX();
  verifyTime += Timer::end("V.x");

  // V: verify the proof
  Timer::start("V.verify");
  auto isValid = verifier->verify(proofs, y, x);
  verifyTime += Timer::end("V.verify");

  circuitTime /= 2;
  auto batchCirN = proverCir->gateCount;
  auto batchCirQ = proverCir->linearCount;

  auto pSize = GP_P->toBinary().size();
  auto qSize = GP_Q->toBinary().size();
  double cipherSize = pSize;
  double proofSize = 1.0 * qSize * (proverCir->batchCount + rangeProofCount); // Cm', CRj
  proofSize += pSize * Lj.size();
  proofSize += qSize * commits.size();
  proofSize += qSize * pc.size();
  proofSize += pSize * proofs.size();
  double proofSizePerMsg = proofSize / msgCount;

  cout << endl;
  cout << "==========" << endl;
  cout << "message count: " << proverCir->msgCount << endl;
  cout << "message/batch: " << proverCir->msgPerBatch << endl;
  cout << "batch count: " << proverCir->batchCount << endl;
  cout << "byte length: " << byteLength * 8 << "-bit" << endl;
  cout << "byte length: " << byteLength << " bytes" << endl;
  cout << "message size: " << proverCir->msgSize << " bytes" << endl;
  cout << "slot size: " << proverCir->slotSize << " bytes" << endl;
  cout << "slot/message: " << proverCir->slotsPerMsg << endl;
  cout << "range proof count: " << proverCir->rangeProofCount << endl;
  cout << "cipher size: " << cipherSize << " bytes" << endl;
  cout << "proof size: " << proofSize << " bytes" << endl;
  cout << "proof size/message: " << proofSizePerMsg << " bytes" << endl;

  cout << endl;
  cout << "single encrypt circuit's N: " << encCirN << endl;
  cout << "single encrypt circuit's Q: " << encCirQ << endl;
  cout << "batch encrypt circuit's N: " << batchCirN << endl;
  cout << "batch encrypt circuit's Q: " << batchCirQ << endl;
  cout << "batch encrypt circuit's matrix m: " << m1 << endl;
  cout << "batch encrypt circuit's matrix n: " << n1 << endl;

  cout << endl;
  cout << "encryption time: " << encryptTime << endl;
  cout << "circuit create time: " << circuitTime << endl;
  cout << "value assign time: " << valueTime << endl;
  cout << "commit time: " << commitTime << endl;
  cout << "prove time: " << proveTime << endl;
  cout << "verify time: " << verifyTime << endl;
  cout << "==========" << endl;

  fs << proverCir->msgCount << ",";
  fs << proverCir->msgPerBatch << ",";
  fs << proverCir->batchCount << ",";
  fs << byteLength * 8 << ",";
  fs << byteLength << ",";
  fs << proverCir->msgSize << ",";
  fs << proverCir->slotSize << ",";
  fs << proverCir->slotsPerMsg << ",";
  fs << proverCir->rangeProofCount << ",";
  fs << cipherSize << ",";
  fs << proofSize << ",";
  fs << proofSizePerMsg << ",";
  fs << encCirN << ",";
  fs << encCirQ << ",";
  fs << batchCirN << ",";
  fs << batchCirQ << ",";
  fs << m1 << ",";
  fs << n1 << ",";
  fs << encryptTime << ",";
  fs << circuitTime << ",";
  fs << valueTime << ",";
  fs << commitTime << ",";
  fs << proveTime << ",";
  fs << verifyTime << endl;

  if (!isValid)
    throw invalid_argument("zkp is not valid");
}
