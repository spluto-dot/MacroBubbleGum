
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

// Struct to represent a single frame of inputs
struct Frame {
    int frame_number;
    std::vector<std::string> keys_pressed;
};

// Class to handle input simulation and frame playback
class InputSimulator {
public:
    InputSimulator(int fps) : frames_per_second(fps) {}

    void addInput(int frame_number, const std::string& key) {
        ensureFrameExists(frame_number);
        frames[frame_number].keys_pressed.push_back(key);
    }

    void playback() {
        for (const auto& frame : frames) {
            std::cout << "Frame " << frame.frame_number << ": ";
            for (const auto& key : frame.keys_pressed) {
                std::cout << key << " ";
            }
            std::cout << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / frames_per_second));
        }
    }

    void setFPS(int fps) {
        frames_per_second = fps;
    }

private:
    std::vector<Frame> frames;
    int frames_per_second;

    void ensureFrameExists(int frame_number) {
        if (frames.size() <= frame_number) {
            frames.resize(frame_number + 1);
            frames[frame_number].frame_number = frame_number;
        }
    }
};

// Main program loop
int main() {
    int fps = 60; // default to 60 FPS
    InputSimulator simulator(fps);

    simulator.addInput(2, "U");
    simulator.addInput(3, "D");
    simulator.addInput(4, "L");
    simulator.addInput(5, "R");

    std::cout << "Starting playback at " << fps << " FPS..." << std::endl;
    simulator.playback();

    return 0;
}
