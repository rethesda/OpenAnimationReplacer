#pragma once

class AnimationFileHashCache final
{
public:
	static AnimationFileHashCache& GetSingleton()
	{
		static AnimationFileHashCache singleton;
		return singleton;
	}

	static std::string CalculateHash(std::string_view a_fullPath);

	[[nodiscard]] bool TryGetCachedHash(std::string_view a_path, std::string& a_outCachedHash) const;

	void SaveHash(std::string_view a_path, std::string_view a_hash)
	{
		WriteLocker locker(_dataLock);

		_cache.emplace(a_path, a_hash);
	}

private:
	AnimationFileHashCache() = default;
	AnimationFileHashCache(const AnimationFileHashCache&) = delete;
	AnimationFileHashCache(AnimationFileHashCache&&) = delete;
	~AnimationFileHashCache() = default;

	AnimationFileHashCache& operator=(const AnimationFileHashCache&) = delete;
	AnimationFileHashCache& operator=(AnimationFileHashCache&&) = delete;

	mutable SharedLock _dataLock;
	std::unordered_map<std::string, std::string> _cache;
};
