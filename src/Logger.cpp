
#include "../include/Logger.h"

namespace hyperion::utils {
	std::atomic_bool Logger::INITIALIZED = std::atomic_bool(false);
	std::atomic_bool Logger::EXIT_THREAD = std::atomic_bool(false);
	[[clang::no_destroy]] std::string Logger::NAME_ROOT = ""s; // NOLINT
	// std::shared_ptr<juce::FileLogger> Logger::LOGGER = nullptr;
	[[clang::no_destroy]] std::shared_ptr<LockFreeQueue<std::string>> Logger::MESSAGES = nullptr;
	[[clang::no_destroy]] std::unique_ptr<std::thread> Logger::MESSAGE_THREAD = nullptr;

} // namespace hyperion::utils
