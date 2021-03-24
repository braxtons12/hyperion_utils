#pragma once

#include <atomic>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <thread>

#include "LockFreeQueue.h"
#include "synchronization/ReadWriteLock.h"

namespace hyperion::utils {
	class Logger {
	  public:
		static auto log_message(std::string message) noexcept -> void {
			auto local = std::move(message);
			if(INITIALIZED.load()) {
				auto messages = get_messages_instance();
				if(messages != nullptr) {
					auto res = messages->push(local);
					if(res.is_err()) {
						std::cerr << "Failed to log Message: " << local << "\n";
					}
				}
			}
		}

		static auto init_logger(std::string rootName) noexcept -> void {
			auto initialized = false;
			if(INITIALIZED.compare_exchange_strong(initialized, true, std::memory_order_seq_cst)) {
				EXIT_THREAD.store(false);
				NAME_ROOT = std::move(rootName);
				if(MESSAGE_THREAD == nullptr) {
					MESSAGE_THREAD = std::make_unique<std::thread>([&]() {
						while(!EXIT_THREAD.load()) {
							auto messages = get_messages_instance();
							if(!messages->isEmpty()) {
								// TODO(braxtons12): convert this to use our own (future) logger
								// auto logger = get_logger_instance();
								// if(logger != nullptr) {
								//	auto res = messages->read();
								//	if(res.is_ok()) {
								//		logger->logMessage(res.unwrap());
								//	}
								//}
							}
						}
					});
				}
				INITIALIZED.store(true);
			}
		}

		static auto close_logger() noexcept -> void {
			auto initialized = true;
			if(INITIALIZED.compare_exchange_strong(initialized, false, std::memory_order_seq_cst)) {
				EXIT_THREAD.store(true);
				MESSAGE_THREAD->join();
				INITIALIZED.store(false);
			}
		}

		static auto logger_initialized() noexcept -> bool {
			return INITIALIZED.load();
		}

	  private:
		static std::atomic_bool INITIALIZED;
		static std::atomic_bool EXIT_THREAD;
		static std::string NAME_ROOT;
		// TODO(braxtons12): convert this to use our own (future) logger
		// static std::shared_ptr<juce::FileLogger> LOGGER;
		static std::shared_ptr<LockFreeQueue<std::string>> MESSAGES;
		static std::unique_ptr<std::thread> MESSAGE_THREAD;

		// TODO(braxtons12): convert this to use our own (future) logger
		// static auto get_logger_instance() noexcept -> std::shared_ptr<juce::FileLogger> {
		//	if(LOGGER == nullptr) {
		//		LOGGER = std::shared_ptr<juce::FileLogger>(
		//			juce::FileLogger::createDateStampedLogger("ApexAudio",
		//													  NAME_ROOT,
		//													  ".hyperionlog",
		//													  "Starting Apex Audio Logger"));
		//	}
		//	return LOGGER;
		// }

		static auto
		get_messages_instance() noexcept -> std::shared_ptr<LockFreeQueue<std::string>> {
			if(MESSAGES == nullptr) {
				MESSAGES = std::make_shared<LockFreeQueue<std::string>>();
			}

			return MESSAGES;
		}
	};
} // namespace hyperion::utils
