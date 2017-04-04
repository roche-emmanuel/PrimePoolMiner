#include "core.h"
#include "config.h"
#include <deque>
#include <numeric>

#include "oacc/AccSieve.h"

#pragma GCC optimize ("unroll-loops")

volatile unsigned int nBestHeight = 0;
unsigned int nStartTimer = 0;
volatile uint64 nWeight = 0, nElements = 0, nPrimes = 0;
double nAverage = 0;
volatile unsigned int nLargestShare = 0;
unsigned int nDifficulty   = 0;
volatile uint64 sieveTime = 0;
volatile uint64 pTestTime = 0;
volatile uint64 sieveCount = 0;
volatile uint64 testCount = 0;
volatile uint64 shareCount = 0;
volatile uint64 candidateCount = 0;
volatile uint64 candidateHitCount = 0;
volatile uint64 candidateHit2Count = 0;
volatile uint64 totalShareWeight = 0;
volatile uint32_t chainCounter[10] = { 0 };


std::string ADDRESS;
uint64 nCurrentPayout = 0, nAccountBalance = 0;

static LLP::Timer cPrimeTimer;
std::deque<double> vPPSValues;
std::deque<double> vWPSValues;

namespace Core
{
	/** Main Miner Thread. Bound to the class with boost. Might take some rearranging to get working with OpenCL. **/
	void MinerThread::PrimeMiner()
	{
		//unsigned long candidates[MAXCANDIDATESPERSIEVE];
		printf("Starting Prime Miner thread\n");
		loop
		{
			try
			{
				/** Keep thread at idle CPU usage if waiting to submit or recieve block. **/
				Sleep(1);
				
				/** Assure that this thread stays idle when waiting for new block, or share submission. **/
				if(fNewBlock || fBlockWaiting || !fNewBlockRestart)
					continue;
				
                {
                    LOCK(MUTEX);
                    fNewBlockRestart = false;
                }
                
				/** Lock the Thread at this Mutex when Changing Block Pointer. **/
				CBigNum BaseHash(hashPrimeOrigin);
				mpz_t zPrimeOrigin, zPrimeOriginOffset, zFirstSieveElement, zPrimorialMod, zTempVar, zResidue, zTwo, zN, zOctuplet;
						
				unsigned int i = 0;
				unsigned int j = 0;
				unsigned int nSize = 0;
				unsigned int nPrimeCount = 0;
				//unsigned int nSieveDifficulty = 0;
				uint64 nStart = 0;
				uint64 nStop = 0;
				unsigned int nLastOffset = 0;
				
				#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
					uint64 nNonce = 0;
				#else
					unsigned long nNonce = 0;
				#endif
				
				//long nElapsedTime = 0;
				//long nStartTime = 0;
				mpz_init(zPrimeOriginOffset);
				mpz_init(zFirstSieveElement);
				mpz_init(zPrimorialMod);
				mpz_init(zOctuplet);
				mpz_init(zTempVar);
				mpz_init(zPrimeOrigin);
				mpz_init(zResidue);
				mpz_init_set_ui(zTwo, 2);
				mpz_init(zN);

				bignum2mpz(&BaseHash, zPrimeOrigin);
				
				nSize = mpz_sizeinbase(zPrimeOrigin, 2);
				//unsigned int* bit_array_sieve = (unsigned int*)malloc((nBitArray_Size)/8);
				//unsigned int * bit_array_sieve = (unsigned int *)aligned_alloc(64, (nBitArray_Size) / 8);
				for(j=0; j < 153 && !fNewBlockRestart; j++)
				{
					/*
					memset(bit_array_sieve, 0x00, (nBitArray_Size)/8);

					mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
					mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);

					mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);
					
					#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
						mpz_import(zOctuplet, 1, 1, sizeof(octuplet_origins[j]), 0, 0, &octuplet_origins[j]);
						mpz_add(zPrimorialMod, zPrimorialMod, zOctuplet);
					#else
						mpz_add_ui(zPrimorialMod, zPrimorialMod, octuplet_origins[j]);
					#endif

					
					mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);
					mpz_set(zFirstSieveElement, zTempVar);

					for(unsigned int i=nPrimorialEndPrime; i<nPrimeLimit && !fNewBlockRestart; i++)
					{
						unsigned long  p = primes[i];
						unsigned int inv = inverses[i];
						unsigned int base_remainder = mpz_tdiv_ui(zTempVar, p);

						unsigned int remainder = base_remainder;
						unsigned long r = (p-remainder)*inv;
						unsigned int index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index)>>5] |= (1<<((index)&31));
							index += p;
						}
							
						remainder = base_remainder + 2;
						if (p<remainder)
							remainder -= p;
						r = (p-remainder)*inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index) >> 5] |= (1 << ((index) & 31));
							index += p;
						}

						remainder = base_remainder + 6;
						if (p<remainder)
							remainder -= p;
						r = (p-remainder)*inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index) >> 5] |= (1 << ((index) & 31));
							index += p;
						}


						remainder = base_remainder + 8;
						if (p<remainder)
							remainder -= p;
						r = (p - remainder) * inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index) >> 5] |= (1 << ((index) & 31));
							index += p;
						}
						
						remainder = base_remainder + 12;
						if (p<remainder)
							remainder -= p;
						r = (p - remainder) * inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index) >> 5] |= (1 << ((index) & 31));
							index += p;
						}
					}
					*/
					int jobId = 0;
					while (!pcJobQueuePassive.pop(jobId))
					{
						printf("Test queue is full!!!!!!\n");
						Sleep(200);

					}
					primeTestJob job = cServerConnection->primeTestJobs.at(jobId);
					job.baseHash->setuint1024(BaseHash.getuint1024());

					int64 nStartTime = GetTimeMicros();
					uint64 tupleOrigin = tentuplet2_origins[j];
					pgisieve(NULL, nBitArray_Size, zPrimorial, zPrimeOrigin, tupleOrigin, primes, inverses, nPrimorialEndPrime, nPrimeLimit, &zFirstSieveElement, job.candidates);
					int64 nAfterSieve = GetTimeMicros();

					mpz_set(job.zFirstSieveElement, zFirstSieveElement);
					mpz_set(job.zPrimeOrigin, zPrimeOrigin);
										
					pcJobQueueActive.push(jobId);

					int64 nElapsedTime = nAfterSieve - nStartTime;
					sieveTime += nElapsedTime;
					sieveCount++;					
					/*
					std::vector<unsigned long>  nonces;
					
					nPrimes += find_tuples(job.candidates, zPrimorial, zPrimeOrigin, zFirstSieveElement, 3, &nonces);
					nElapsedTime = GetTimeMicros() - nAfterSieve;
					pTestTime += nElapsedTime;

					//check_candidates(bit_array_sieve, nBitArray_Size, zPrimorial, zPrimeOrigin, zFirstSieveElement, 3, &nonces);
					for (std::vector<unsigned long>::iterator it = nonces.begin(); it != nonces.end(); ++it)
					{
						nNonce = *it;
						unsigned int nDiff = GetPrimeBits((BaseHash + nNonce), 1);

						nWeight += nDiff * 50;

						if (nDiff > nLargestShare)
							nLargestShare = nDiff;

						if (nDiff < nMinimumShare)
							continue;

						if (nDiff >= nMinimumShare)
						{
							printf("Submitting share %ul\n", nDiff);
							cServerConnection->SubmitShare(BaseHash.getuint1024(), nNonce);
						}
					}
					
					*/
					/*
					for(i=0; i<nBitArray_Size && !fNewBlockRestart; i++)
					{
						if( bit_array_sieve[(i)>>3] & (1<<((i)&7)) )
							continue;

						// p1
						mpz_mul_ui(zTempVar, zPrimorial, i);
						mpz_add(zTempVar, zFirstSieveElement, zTempVar);
						mpz_set(zPrimeOriginOffset, zTempVar);

						mpz_sub_ui(zN, zTempVar, 1);
						mpz_powm(zResidue, zTwo, zN, zTempVar);
						if (mpz_cmp_ui(zResidue, 1) != 0)
							continue;

						nStart = 2;
						nStop = 2;
						nPrimeCount = 1;
						nLastOffset = 2;
						nPrimes+=2;

                         mpz_add_ui(zTempVar, zTempVar, 2);
						for(nStart; nStart <= nStop + 12 && !fNewBlockRestart; nStart += 2)
						{
							mpz_sub_ui(zN, zTempVar, 1);
							mpz_powm(zResidue, zTwo, zN, zTempVar);
							if (mpz_cmp_ui(zResidue, 1) == 0)
							{
								nStop = nStart;
								nPrimeCount++;
								nPrimes++;
								//nPrimesForSieve++;
							}

							mpz_add_ui(zTempVar, zTempVar, 2);
							
							nLastOffset += 2;
						}

						if(nPrimeCount >= 3 && !fNewBlockRestart)
						{   
							mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);
							
							#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
								nNonce = mpz2uint64(zTempVar);
							#else
								nNonce = mpz_get_ui(zTempVar);
							#endif
													
							unsigned int nSieveBits = SetBits(GetSieveDifficulty(BaseHash + nNonce + nLastOffset, nPrimeCount));
							if(nSieveBits > nLargestShare)
								nLargestShare = nSieveBits;

							unsigned int nDiff = GetPrimeBits((BaseHash + nNonce), 1);

							nWeight += nDiff * 50;

							if(nSieveBits < nMinimumShare)
								continue;

							if(nDiff >= nMinimumShare)
							{
								cServerConnection->SubmitShare(BaseHash.getuint1024(), nNonce);
							}
						}
					}
					*/
				}
				
				mpz_clear(zPrimeOrigin);
				mpz_clear(zOctuplet);
				mpz_clear(zPrimeOriginOffset);
				mpz_clear(zFirstSieveElement);
				mpz_clear(zResidue);
				mpz_clear(zTwo);
				mpz_clear(zN);
				mpz_clear(zPrimorialMod);
				mpz_clear(zTempVar);

				//free(bit_array_sieve);

                if( !fNewBlockRestart && !fBlockWaiting )
				    fNewBlock = true;
			}
			catch(std::exception& e){ printf("ERROR: %s\n", e.what()); }
		}
	}
	

	/** Reset the block on each of the Threads. **/
	void ServerConnection::ResetThreads()
	{
		
		int dummy;
		while (pcJobQueueActive.pop(dummy))
		{
		}

		while (pcJobQueuePassive.pop(dummy))
		{
		}
		for (size_t i = 0; i < 1000; i++)
		{
			pcJobQueuePassive.push(i);
		}
		/** Clear the Submit Queue. **/
		SUBMIT_MUTEX.lock();
		
		while(!SUBMIT_QUEUE.empty())
			SUBMIT_QUEUE.pop();
			
		SUBMIT_MUTEX.unlock();
		
		while(!RESPONSE_QUEUE.empty())
			RESPONSE_QUEUE.pop();
		
		/** Reset each individual flag to tell threads to stop mining. **/
		for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
        {
            LOCK(THREADS[nIndex]->MUTEX);
			THREADS[nIndex]->fNewBlock      = true;		
            THREADS[nIndex]->fNewBlockRestart = true;	
        }
	}
		
	/** Add a Block to the Submit Queue. **/
	void ServerConnection::SubmitShare(uint1024 hashPrimeOrigin, uint64 nNonce)
	{
		SUBMIT_MUTEX.lock();
		SUBMIT_QUEUE.push(std::make_pair(hashPrimeOrigin, nNonce));
		SUBMIT_MUTEX.unlock();
	}
		
		
	/** Main Connection Thread. Handles all the networking to allow
		Mining threads the most performance. **/
	void ServerConnection::ServerThread()
	{
		
		/** Don't begin until all mining threads are Created. **/
		//while(THREADS.size() != nThreads)
		//	Sleep(1);
				
		int maxChToPrint = 9;

		/** Initialize the Server Connection. **/
		CLIENT = new LLP::Miner(IP, PORT);
			
				
		/** Initialize a Timer for the Hash Meter. **/
		TIMER.Start();
		cPrimeTimer.Start();
			
		//unsigned int nBestHeight = 0;
		loop
		{
			try
			{
				/** Run this thread at 100 Cycles per Second. **/
				Sleep(10);
					
					
				/** Attempt with best efforts to keep the Connection Alive. **/
				if(!CLIENT->Connected() || CLIENT->Errors() || CLIENT->Timeout(30))
				{
					if(!CLIENT->Connect())
						continue;
					else
					{
						CLIENT->Login(ADDRESS);
							
						CLIENT->GetBalance();
						CLIENT->GetPayouts();
						
						ResetThreads();
					}
				}

				if( cPrimeTimer.Elapsed() >= 1)
				{
					unsigned int nElapsed = cPrimeTimer.Elapsed();
					
					double PPS = (double) nPrimes / (double)(nElapsed );
					double WPS = (double)nWeight / (double)(nElapsed * 10000000);

					if( vPPSValues.size() >= 300)
						vPPSValues.pop_front();
					
					vPPSValues.push_back(PPS);

					if( vWPSValues.size() >= 300)
						vWPSValues.pop_front();
					
					vWPSValues.push_back(WPS);
					
					nPrimes = 0;
					nWeight = 0;
					cPrimeTimer.Reset();
				}
					
				/** Rudimentary Meter **/
				if(TIMER.Elapsed() >= 5)
				{
					if (sieveCount == 0)
						sieveCount++;
					nElements++;
					
					unsigned int SecondsElapsed = (unsigned int)time(0) - nStartTimer;
					unsigned int nElapsed = TIMER.Elapsed();
					uint64 avgSieveTime = sieveTime / sieveCount;
					double SPS = (double)sieveCount / (double)SecondsElapsed;
					double TPS = (double)testCount / (double)SecondsElapsed;
					double PPS = 1.0 * std::accumulate(vPPSValues.begin(), vPPSValues.end(), 0LL) / vPPSValues.size();
					double WPS = 1.0 * std::accumulate(vWPSValues.begin(), vWPSValues.end(), 0LL) / vWPSValues.size();;
					struct tm t;
					time_t aclock = SecondsElapsed;
					gmtime_r(&aclock, &t);

					printf("[METERS] %f WPS | Largest Share %f | Diff. %f | Height = %u | Balance: %f NXS | Payout: %f NXS | %02dd-%02d:%02d:%02d\n", WPS, nLargestShare / 10000000.0, nDifficulty / 10000000.0, nBestHeight, nAccountBalance / 1000000.0, nCurrentPayout / 1000000.0, t.tm_yday, t.tm_hour, t.tm_min, t.tm_sec);
					
					CLIENT->SubmitPPS(PPS, WPS);

					printf( "-----------------------------------------------------------------------------------------------\nch  \t| ");
					for (int i = 3; i <= maxChToPrint; i++)
						printf( "%-7d  |  ", i);
					printf( "\n---------------------------------------------------------------------------------------------\ncount\t| ");
					for (int i = 3; i <= maxChToPrint; i++)
						printf( "%-7d  |  ", chainCounter[i]);
					printf( "\n---------------------------------------------------------------------------------------------\nch/m\t| ");
					for (int i = 3; i <= maxChToPrint; i++)
					{
						double sharePerHour = ((double)chainCounter[i] / SecondsElapsed) * 60.0;
						printf( "%-7.03f  |  ", sharePerHour);
					}
					printf( "\n--------------------------------------------------------------------------------------------\nratio\t| ");
					for (int i = 3; i <= maxChToPrint; i++)
					{
						double chRatio = 0;
						if (chainCounter[i] != 0)
							chRatio = ((double)chainCounter[i - 1] / (double)chainCounter[i]);
						printf( "%-7.03f  |  ", chRatio);
					}

					double sharePerH = ((double)shareCount / SecondsElapsed) * 3600;
					double shareWeightPerH = ((double)totalShareWeight / SecondsElapsed) * 3.6;
					printf("\n---------------------------------------------------------------------------------------------\n");
					printf("AVG Time - Sieve: %u - PTest: %u  - Siv/s: %-7.03f - Tst/s: %-7.03f - Shares: %u - Shr/h: %-7.03f - TotalShareValue: %u - ShrVal/h: %-5.02fK\n", \
						avgSieveTime, testCount == 0 ? 0 : pTestTime / testCount, SPS, TPS, shareCount, sharePerH, totalShareWeight, shareWeightPerH);
			
					printf("AvgCandCnt: %u - CandHit : %-2.02f%% 2nd : %-2.02f%%\n\n", \
						testCount == 0 ? 0 : candidateCount / testCount, \
						candidateCount == 0 ? 0 : (double)(candidateHitCount * 100) / (double)candidateCount, \
						candidateCount == 0 ? 0 : (double)(candidateHit2Count * 100) / (double)candidateCount);
					
					TIMER.Reset();
				}
					
					
				/** Submit any Shares from the Mining Threads. **/
				SUBMIT_MUTEX.lock();
				while(!SUBMIT_QUEUE.empty())
				{
					std::pair<uint1024, uint64> pShare = SUBMIT_QUEUE.front();
					SUBMIT_QUEUE.pop();
					
					CLIENT->SubmitShare(pShare.first, pShare.second);
					RESPONSE_QUEUE.push(pShare);
				}
				SUBMIT_MUTEX.unlock();
				
				
				/** Check if there is work to do for each Miner Thread. **/
				for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
				{
					/** Attempt to get a new block from the Server if Thread needs One. **/
					if(THREADS[nIndex]->fNewBlock)
					{
						CLIENT->GetBlock();
						THREADS[nIndex]->fBlockWaiting = true;
						THREADS[nIndex]->fNewBlock = false;
                        THREADS[nIndex]->fNewBlockRestart = true;
						
						//printf("[MASTER] Asking For New Block for Thread %u\n", nIndex);
					}
				}
					
				CLIENT->ReadPacket();
				if(!CLIENT->PacketComplete())
					continue;
						
				/** Handle the New Packet, and Interpret its Data. **/
				LLP::Packet PACKET = CLIENT->NewPacket();
				CLIENT->ResetPacket();
							
							
				/** Output if a Share is Accepted. **/
				if(PACKET.HEADER == CLIENT->ACCEPT)
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					double nDiff = GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1);
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Accepted]\n", nDiff, pResponse.first.ToString().substr(0, 20).c_str());
					totalShareWeight += pow(13.0, nDiff - 2.0);
				}
					
					
				/** Output if a Share is Rejected. **/
				else if(PACKET.HEADER == CLIENT->REJECT) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Rejected]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
				}
					
				/** Output if a Share is a Block **/
				else if(PACKET.HEADER == CLIENT->BLOCK) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("\n******************************************************\n\nBlock Accepted | Difficulty %f | Hash %s\n\n******************************************************\n\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().c_str());
				}
					
				/** Output if a Share is Stale **/
				else if(PACKET.HEADER == CLIENT->STALE) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Stale]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
				}
					
					
				/** Reset the Threads if a New Block came in. **/
				else if(PACKET.HEADER == CLIENT->NEW_BLOCK)
				{
					printf("[MASTER] NXS Network: New Block\n");
							
					ResetThreads();
					
					CLIENT->GetBalance();
					CLIENT->GetPayouts();
				}
					
				/** Set the Current Account Balance if Message Received. **/
				else if(PACKET.HEADER == CLIENT->ACCOUNT_BALANCE) { nAccountBalance = bytes2uint64(PACKET.DATA); }
					
					
				/** Set the Current Pending Payout if Message Received. **/
				else if(PACKET.HEADER == CLIENT->PENDING_PAYOUT) { nCurrentPayout = bytes2uint64(PACKET.DATA); }
					
					
				/** Set the Block for the Thread if there is a New Block Packet. **/
				else if(PACKET.HEADER == CLIENT->BLOCK_DATA)
				{
					/** Search for a Thread waiting for a New Block to Supply its need. **/
					for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
					{
						if(THREADS[nIndex]->fBlockWaiting)
						{
							LOCK(THREADS[nIndex]->MUTEX);
							
							THREADS[nIndex]->hashPrimeOrigin.SetBytes(std::vector<unsigned char>  (PACKET.DATA.begin(),   PACKET.DATA.end() - 12));
							nMinimumShare = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 12, PACKET.DATA.end() - 8));
							THREADS[nIndex]->nMinimumShare = nMinimumShare;
		
								
							/** Check that the Block Received is not Obsolete, if so request a new one. **/
							unsigned int nHeight = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 4, PACKET.DATA.end()));
							if(nHeight < nBestHeight)
							{
								printf("[MASTER] Received Obsolete Block %u... Requesting New Block.\n", nHeight);
								CLIENT->GetBlock();
									
								break;
							}
							
							nDifficulty   = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 8,  PACKET.DATA.end() - 4));
							nBestHeight   = nHeight;
								
							printf("[MASTER] Block %s Height = %u Received on Thread %u\n", THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nHeight, nIndex);
							THREADS[nIndex]->fBlockWaiting = false;
								
							break;
						}
					}
				}
					
			}
			catch(std::exception& e)
			{
				printf("%s\n", e.what()); CLIENT = new LLP::Miner(IP, PORT); 
			}
		}
	}

	int find_tuples2(unsigned long * candidates, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * nonces)
	{
		mpz_t zPrimeOriginOffset, zTempVar, zTempVar2;
		mpz_init(zPrimeOriginOffset);
		mpz_init(zTempVar);
		mpz_init(zTempVar2);
		int n1stPrimeSearchLimit = 0;
		int cx = 0;
		int nPrimes = 0;

		while (candidates[cx] != -1 && cx < MAXCANDIDATESPERSIEVE)
		{
			mpz_mul_ui(zTempVar, zPrimorial, candidates[cx]);
			cx++;
			mpz_add(zTempVar, zFirstSieveElement, zTempVar);
			mpz_set(zPrimeOriginOffset, zTempVar);

			unsigned long long nNonce = 0;
			unsigned int nPrimeCount = 0;
			unsigned int nSieveDifficulty = 0;
			unsigned long nStart = 0;
			unsigned long nStop = 0;
			unsigned long nLastOffset = 0;
			int firstPrimeAt = -1;

			mpz_add_ui(zTempVar2, zTempVar, 18);
			if (mpz_probab_prime_p(zTempVar2, 0) > 0)
				n1stPrimeSearchLimit = 12;
			else
//				continue;
			{ //!!!!! 6
				mpz_add_ui(zTempVar2, zTempVar, 20);
				if (mpz_probab_prime_p(zTempVar2, 0) > 0)
				{
					candidateHit2Count++;
					n1stPrimeSearchLimit = 12;
				}
				else
					//{
					//	mpz_add_ui(zTempVar2, zTempVar, 26);
					//	if (mpz_probab_prime_p(zTempVar2, 0) > 0)
					//		n1stPrimeSearchLimit = 26;
					//	else
					//		continue;
					//}
					continue;
			}
			candidateHitCount++;

			nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
			double diff = 0;
			
			for (nStart = 0; nStart <= nStop + 12; nStart += 2)
			{
				if (nStart == 14 || nStart == 22)//|| nStart == 10 || nStart == 14 || nStart == 16 || nStart == 24)
				{
					mpz_add_ui(zTempVar, zTempVar, 2);
					nLastOffset += 2;
					continue;
				}

				if (mpz_probab_prime_p(zTempVar, 0) > 0)
				{
					nStop = nStart;
					nPrimeCount++;
					nPrimes++;
				}
				if (nPrimeCount == 0 && nStart >= n1stPrimeSearchLimit)
					break;

				if ((firstPrimeAt == -1 && nPrimeCount == 1))
				{
					mpz_set(zPrimeOriginOffset, zTempVar); // zPrimeOriginOffset = zTempVar
					firstPrimeAt = nStart;
				}

				mpz_add_ui(zTempVar, zTempVar, 2);
				nLastOffset += 2;
			}

			if (nPrimeCount >= nMinimumPrimeCount)
			{
				mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
				nNonce = mpz2uint64(zTempVar);
#else
				nNonce = mpz_get_ui(zTempVar);
#endif
				nonces->push_back(nNonce);
			}
		}
		candidateCount += cx;
		//if (nonces->size() > 0)
		//	printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
		return nPrimes;
	}


	void ServerConnection::PrimeTestThread()
	{
		printf("Starting Prime test thread!\n");
		loop
		{
			int jobId = 0;
			if (pcJobQueueActive.pop(jobId))
			{				
			
				//printf("got job id %u on thread %u\n", jobId, boost::this_thread::get_id());

				primeTestJob job = primeTestJobs.at(jobId);

				std::vector<unsigned long>  nonces;
				int64 nStartTime = GetTimeMicros();
				nPrimes += find_tuples2(job.candidates, zPrimorial, job.zPrimeOrigin, job.zFirstSieveElement, 3, &nonces);
				pTestTime += (GetTimeMicros() - nStartTime);
				testCount++;
				pcJobQueuePassive.push(jobId);

				for (std::vector<unsigned long>::iterator it = nonces.begin(); it != nonces.end(); ++it)
				{
					unsigned long nNonce = *it;
					unsigned int nDiff = GetPrimeBits((*job.baseHash + nNonce), 1);
					uint32_t nPrimeCount = nDiff / 10000000;
					//printf(" - %f - Nonce: %u\n", (double)nDiff / 10000000.0, nNonce);
					if (nPrimeCount <10)
					chainCounter[nPrimeCount]++;
					nWeight += nDiff * 50;

					if (nDiff > nLargestShare)
						nLargestShare = nDiff;

					if (nDiff < nMinimumShare)
						continue;

					if (nDiff >= nMinimumShare)
					{
						printf("Submitting share %ul\n", nDiff);
						this->SubmitShare(job.baseHash->getuint1024(), nNonce);
						shareCount++;
					}
				}


			}
			else
				Sleep(10);
		}
	}

}

int main(int argc, char *argv[])
{
	// HashToBeWild!:
	std::string IP = "";
	std::string PORT = "";
	ADDRESS = "";
	
	for (int i = 0; i < sizeof(chainCounter) / sizeof(uint32_t); i++)
	{
		chainCounter[i] = 0;
	}


	Core::MinerConfig Config;
	if(Config.ReadConfig())
	{
		printf("Using the config file...\n");
		// populate settings from config object
		IP = Config.strHost;
		PORT = Config.nPort;
		ADDRESS = Config.strNxsAddress;
	}
	else
	{
		printf("Config file not available... using command line\n");
		if(argc < 4)
		{
			printf("Too Few Arguments. The Required Arguments are 'IP PORT ADDRESS'\n");
			printf("Default Arguments are Total Threads = CPU Cores and Connection Timeout = 10 Seconds\n");
			printf("Format for Arguments is 'IP PORT ADDRESS THREADS TIMEOUT'\n");		
			Sleep(10000);		
			return 0;
		}		
	}
	
	// Command line overrides the config file
	if (argc > 3)
	{
		IP = argv[1];
		PORT = argv[2];
		ADDRESS = argv[3];		
	}

	int nSieveThreads = GetTotalCores() - 1;
	int nPTestThreads = GetTotalCores() + 1;
	int nTimeout = 2;
	
	if(argc > 4)
	{
		nSieveThreads = boost::lexical_cast<int>(argv[4]);
	}
	else
	{
		if (Config.nSieveThreads > 0)
		{
			nSieveThreads = Config.nSieveThreads ;
		}
	}
	
	if(argc > 5)
	{
		nPTestThreads = boost::lexical_cast<int>(argv[5]);
	}
	else
	{
		if (Config.nPTestThreads > 0)
		{
			nPTestThreads = Config.nPTestThreads;
		}
	}

	if (argc > 6)
	{
		nTimeout = boost::lexical_cast<int>(argv[6]);
	}
	else
	{
		if (Config.nTimeout > 0)
		{
			nTimeout = Config.nTimeout;
		}
	}


	printf("Nexus (Coinshield) Prime Pool Miner 1.0.1 - Created by Videlicet - Optimized by Supercomputing extended by paulscreen and hashtobewild \n");
	printf("Using Supplied Account Address %s\n", ADDRESS.c_str());
	printf("Initializing Miner \n");
	printf("Host %s\n", IP.c_str());
	printf("Port: %s\n", PORT.c_str());
	printf("SieveThreads: %i\n", nSieveThreads);
	printf("Prime Test Threads: %i\n", nPTestThreads);
	printf("Timeout = %i\n\n", nTimeout);
	
	Core::InitializePrimes();
	
	
	
	nStartTimer = (unsigned int)time(0);
	Core::nBitArray_Size = Config.nBitArraySize ;
	Core::prime_limit = Config.primeLimit ;
	Core::nPrimeLimit = Config.nPrimeLimit ;
	Core::nPrimorialEndPrime = Config.nPrimorialEndPrime ;

	Core::ServerConnection MINERS(IP, PORT, nSieveThreads, nPTestThreads, nTimeout);
	
	do
	{
		char ch = getchar();
		if (ch == 'q')
			break;
		printf("Enter 'q' to quit\n");
	} while (true);
	//loop { Sleep(10); }
	
	return 0;
}
