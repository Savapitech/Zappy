#pragma once

#include "INetworkClient.hpp"
#include <string>
#include <vector>

namespace Zappy {

class TcpClient : public INetworkClient {
public:
  TcpClient();
  ~TcpClient() override;

  bool connectToServer(const std::string &host, int port) override;
  void sendCommand(const std::string &cmd) override;
  std::vector<std::string> fetchLines() override;

  bool isConnected() const { return _isConnected; }

private:
  int _socket;
  bool _isConnected;
  std::string _buffer;
};

} // namespace Zappy