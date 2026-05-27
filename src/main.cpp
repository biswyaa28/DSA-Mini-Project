#include "api_server.hpp"

int main() {
  ApiServer server;
  server.configure(8080);
  server.run();
  return 0;
}
