#include "AnimationFileHashCache.h"

#include <mmio/mmio.hpp>

#include <xxhash.h>

#include "Parsing.h"

constexpr bool bUse128BitAnimationFileHash = true;

std::string AnimationFileHashCache::CalculateHash(std::string_view a_fullPath)
{
	Parsing::ScopedTimer timer(Parsing::TimingBucket::kAnimationFileHash);

	auto& hashCache = GetSingleton();

	std::string ret;
	if (hashCache.TryGetCachedHash(a_fullPath, ret)) {
		Parsing::AddTimingCounter(Parsing::TimingCounter::kAnimationHashCacheHit);
		return ret;
	}

	// Calculate a hash from the animation file
	mmio::mapped_file_source file;
	if (file.open(a_fullPath)) {
		const auto size = static_cast<uint64_t>(file.size());

		if constexpr (bUse128BitAnimationFileHash) {
			const auto hash = XXH3_128bits(file.data(), file.size());
			ret.reserve(sizeof(size) + sizeof(hash.low64) + sizeof(hash.high64));
			ret.append(reinterpret_cast<const char*>(&size), sizeof(size));
			ret.append(reinterpret_cast<const char*>(&hash.low64), sizeof(hash.low64));
			ret.append(reinterpret_cast<const char*>(&hash.high64), sizeof(hash.high64));
		} else {
			const auto hash = XXH3_64bits(file.data(), file.size());
			ret.reserve(sizeof(size) + sizeof(hash));
			ret.append(reinterpret_cast<const char*>(&size), sizeof(size));
			ret.append(reinterpret_cast<const char*>(&hash), sizeof(hash));
		}

		hashCache.SaveHash(a_fullPath, ret);
		Parsing::AddTimingCounter(Parsing::TimingCounter::kAnimationHashCalculated);
	} else {
		Parsing::AddTimingCounter(Parsing::TimingCounter::kAnimationHashFailed);
	}

	return ret;
}

bool AnimationFileHashCache::TryGetCachedHash(const std::string_view a_path, std::string& a_outCachedHash) const
{
	ReadLocker locker(_dataLock);

	if (const auto it = _cache.find(a_path.data()); it != _cache.end()) {
		a_outCachedHash = it->second;
		return true;
	}
	return false;
}
