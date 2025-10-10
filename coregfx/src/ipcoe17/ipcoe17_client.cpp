// ============================================================================
// ⚠️ ARCHITECTURAL WARNING - LEGACY CODE
// ============================================================================
//
// This file contains gRPC client code that VIOLATES the architectural
// separation of concerns for the coregfx library.
//
// ISSUE:
// - coregfx should ONLY handle local asset loading and Vulkan rendering
// - Remote communication (gRPC/HTTP) should be in separate client libraries
// - This code uses gRPC for remote server communication
//
// RECOMMENDATION:
// - Move this file to: apps/foundation/coregfx-stream-client/src/
// - Remove gRPC dependencies from coregfx/CMakeLists.txt
// - Keep coregfx focused on local rendering and asset loading
//
// REFERENCE:
// - See: docs/remote-asset-loading-architecture.md
// - See: coregfx/ARCHITECTURE_ANALYSIS.md
//
// STATUS: Legacy code - marked for refactoring
// ============================================================================

#include <coregfx/ipcoe17/ipcoe17_client.hpp>
#include <coregfx/core/ocean_log.hpp>

namespace Ipcoe17 {
	IO* clientIO = NULL;  // Definition of the global variable
}

void Ipcoe17::createClientIO(std::string_view host, int32_t port)
{	
	if (clientIO == NULL) {
		clientIO = new IO(host, port);
	} else {
		coregfx::warn("Ipcoe17::createClientIO() clientIO already initialized!");
	}
}

bool Ipcoe17::pingServer()
{
	assert(clientIO != NULL);

	auto& io = *clientIO;

	return io.pingServer();
}

bool Ipcoe17::send(const cgfx::UserRequest& request, cgfx::Value* response)
{
	assert(clientIO != NULL);

	auto& io = *clientIO;

	bool success = io.send(request, response);

	if (!success) {
		coregfx::error("Ipcoe17::send() cannot send request");

		coregfx::error(request.DebugString());
	}

	return success;
}

// Definition of the static mutex
std::mutex Ipcoe17::IO::mtx;

/*
Gibt noch eine Nullpointer-Exception
*/
void Ipcoe17::sendWithRetries(const cgfx::UserRequest& request, cgfx::Value* response) {
	
	if (response == nullptr) {
		throw std::invalid_argument("response pointer is null");
	}
	
	std::thread([request, response]() {
		while (true) {
			std::lock_guard<std::mutex> lock(Ipcoe17::IO::mtx); // Lock the mutex to ensure thread-safe access

			if (!Ipcoe17::send(request, response)) {
				coregfx::warn("cannot connect to server. Retry...");
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				continue;
			}
			break;
		}
	}).detach();
}

void Ipcoe17::loadUserStub(std::string_view host, int32_t port, std::string_view userStubId, cgfx::UserStub& userStub)
{
	if (clientIO == NULL) {
		createClientIO(host, port);
	}

	// Prepare request and response
	cgfx::UserRequest request;
	auto* lusr = new cgfx::LoadUserStubRequest();
	lusr->set_userstubid(std::string(userStubId));
	lusr->set_useruuid("7ea9852e-e6cd-4a2a-aec8-69e55f5eac4c");
	request.set_allocated_loaduserstubrequest(lusr);

	cgfx::Value response;

	send(request, &response);

	// ocean::info(response.DebugString());

	if (response.has_userstub_value()) {
		userStub = response.userstub_value();
	} else {
		coregfx::error("Ipcoe17::loadUserStub() cannot load user-stub");
	}
}

cgfx::ValueStreamRequest Ipcoe17::buildJoinRequestMsg()
{
	cgfx::ValueStreamRequest vs;

	auto m = vs.mutable_join();
	(*m).set_useruuid("7ea9852e-e6cd-4a2a-aec8-69e55f5eac4c");
	(*m).set_route("monster-map");
	(*m).set_gameappuuid(std::to_string(2260415837));
	(*m).set_gameappversion("0.1.39");
	(*m).set_gamemodeid("666ef7df3470ae09b4ea276c");

	return vs;
}