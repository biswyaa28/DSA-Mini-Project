#include "api_server.hpp"
#include <cassert>

int main() {
  ApiServer server;
  assert(server.configure(8081));
  return 0;
}
