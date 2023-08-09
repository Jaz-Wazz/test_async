
#include <iofox.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <fmt/core.h>
#include <exception>

namespace asio { using namespace boost::asio; }

auto coro(asio::io_context & executor) -> io::coro<void>
{
	fmt::print("sas.\n");
	co_return;
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(ctx), io::rethrowed);
	ctx.run();
	return 0;
}
catch(const std::exception & exception)
{
	fmt::print("[main] - exception: '{}'.\n", exception.what());
}
