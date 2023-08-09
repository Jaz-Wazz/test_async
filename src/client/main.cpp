
#include <iofox.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/core.h>
#include <exception>
#include <chrono>
#include <string>
#include <vector>

namespace asio { using namespace boost::asio; using namespace boost::asio::experimental; }

auto reader(asio::io_context & executor, asio::channel<void(boost::system::error_code, std::string)> & channel) -> io::coro<void>
{
	asio::ip::tcp::socket socket {executor};
	asio::ip::tcp::endpoint endpoint {asio::ip::make_address("127.0.0.1"), 555};
	co_await socket.async_connect(endpoint, io::use_coro);

	for(;;)
	{
		auto buffer = co_await channel.async_receive(io::use_coro);

		int package_size = buffer.size();
		buffer.insert(0, reinterpret_cast<char *>(&package_size), 4);

		co_await asio::async_write(socket, asio::buffer(buffer), asio::transfer_exactly(buffer.size()), io::use_coro);
		fmt::print("[reader] - read {} bytes and send to server.\n", buffer.size() - 4);
	}
}

auto writer(char name, std::string str, asio::channel<void(boost::system::error_code, std::string)> & channel) -> io::coro<void>
{
	for(;;)
	{
		fmt::print("[writer '{}'] - write: '{}'.\n", name, str);
		co_await channel.async_send({}, str, io::use_coro);
		co_await asio::steady_timer(co_await asio::this_coro::executor, std::chrono::seconds(1)).async_wait(io::use_coro);
	}
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;

	asio::channel<void(boost::system::error_code, std::string)> channel {ctx};
	asio::co_spawn(ctx, reader(ctx, channel), io::rethrowed);
	asio::co_spawn(ctx, writer('x', "garox", channel), io::rethrowed);
	asio::co_spawn(ctx, writer('y', "dudos", channel), io::rethrowed);

	ctx.run();
	return 0;
}
catch(const std::exception & exception)
{
	fmt::print("[main] - exception: '{}'.\n", exception.what());
}
