
#include <Hyperion/error/Panic.h>

namespace hyperion::error {
	std::atomic<Panic::handler_type> Panic::handler = Panic::get_default_handler(); // NOLINT
} // namespace hyperion::error
