// Copyright 2018 The Beam Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core/block_crypt.h"
#include "crypto/equihash.h"
#include "uint256.h"
#include "arith_uint256.h"
#include <utility>
#include "equihash.h"

namespace beam
{

struct Block::PoW::Helper
{
	blake2b_state m_Blake;
	Equihash<Block::PoW::N, Block::PoW::K> m_Eh;

	void Reset(const void* pInput, uint32_t nSizeInput, const NonceType& nonce)
	{
		m_Eh.InitialiseState(m_Blake);

		// H(I||...
		blake2b_update(&m_Blake, (uint8_t*) pInput, nSizeInput);
		blake2b_update(&m_Blake, nonce.m_pData, nonce.nBytes);
	}

	bool TestDifficulty(const uint8_t* pSol, uint32_t nSol, Difficulty d) const
	{
		ECC::Hash::Value hv;
		ECC::Hash::Processor() << Blob(pSol, nSol) >> hv;

		return d.IsTargetReached(hv);
	}
};


bool Block::PoW::IsValidSolution(const void* pInput, uint32_t nSizeInput) const
{
	Helper hlp;
	hlp.Reset(pInput, nSizeInput, m_Nonce);

	std::vector<uint8_t> v(m_Indices.begin(), m_Indices.end());
    return
		hlp.m_Eh.IsValidSolution(hlp.m_Blake, v);
}

bool Block::PoW::IsValid(const void* pInput, uint32_t nSizeInput) const
{
	Helper hlp;
	hlp.Reset(pInput, nSizeInput, m_Nonce);

	std::vector<uint8_t> v(m_Indices.begin(), m_Indices.end());
    return
		hlp.m_Eh.IsValidSolution(hlp.m_Blake, v) &&
		hlp.TestDifficulty(&m_Indices.front(), (uint32_t) m_Indices.size(), m_Difficulty);
}

} // namespace beam

int verifySolution(const char* input, const char* nonce, const char* output)
{
	beam::Block::PoW pow;

	memcpy(pow.m_Indices.data(), output, beam::Block::PoW::nSolutionBytes);
	memcpy(pow.m_Nonce.m_pData, nonce, beam::Block::PoW::NonceType::nBytes);

	return int(pow.IsValidSolution(input, 32));
}
