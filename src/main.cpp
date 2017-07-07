#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

#include <atomic>
#include <thread>

#include <chrono>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  } else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

double kp_g = 0.1, ki_g = 0.00, kd_g = 0.05; // 30mph
// kp ki kd :( 0.100, 0.000, 0.050 ) for 50mph
// kp ki kd :( 0.150, 0.000, 0.060 ) for 30mph
bool clear_error_g = false;
double deadband_g = 0.0;
void readCin(std::atomic<bool> &run) {
  std::string input;
  while (run.load()) {
    std::cout << "> ";
    getline(std::cin, input);
    if (input == "quit")
      run.store(false);
    else if (input[0] == 'c') {
      clear_error_g = true;
    } else {
      double val = std::stod(input.substr(1));
      switch (input[0]) {
      case 'p':
        kp_g = val;
        break;
      case 'i':
        ki_g = val;
        break;
      case 'd':
        kd_g = val;
        break;
      case 't':
        deadband_g = val;
        break;
      }
    }
    printf("kp ki kd :( %.3f, %.3f, %.3f )\n", kp_g, ki_g, kd_g);
  }
}

int main() {
  uWS::Hub h;

  PID pid;
  pid.Init(kp_g, ki_g, kd_g);

  std::atomic<bool> run(true);
  std::thread cin_thread(readCin, std::ref(run));

  auto start = std::chrono::high_resolution_clock::now();

  h.onMessage([&pid, &start](uWS::WebSocket<uWS::SERVER> ws, char *data,
                             size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2') {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {

          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> diff = end - start;
          //          std::cout << "Dt = " << diff.count() << "\n";
          start = end;

          //  std::cout << j << "\n";

          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          // double speed = std::stod(j[1]["speed"].get<std::string>());
          // double angle =
          // std::stod(j[1]["steering_angle"].get<std::string>());
          //          double heading =
          // std::stod(j[1]["psi"].get<std::string>());
          double steer_value;

          // Ingnore the initial inaccurate duration measure
          if (diff.count() > 1.0)
            pid.SetDt(0.05);
          else
            pid.SetDt(diff.count());

          if (clear_error_g) {
            std::cout << "Clear PID errors\n";
            clear_error_g = false;
            pid.ClearError();
          }

          pid.SetPID(kp_g, ki_g, kd_g);
          pid.SetDeadBand(deadband_g);

          steer_value = pid.UpdateError(cte);

          // DEBUG
          // std::cout << "CTE: " << cte << " heading: " << heading <<
          // std::endl;
          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.5;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          // std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the
  // program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data,
                     size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1) {
      res->end(s.data(), s.length());
    } else {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h, &pid](uWS::WebSocket<uWS::SERVER> ws,
                            uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
    pid.ClearError();
    pid.PrintPID();
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();

  run.store(false);
  cin_thread.join();
}
