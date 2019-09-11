/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <fstream>
#include <cstdio>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/audio_stream.grpc.pb.h"
#else
#include "audio_stream.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using audiostream::AudioRequest;
using audiostream::AudioData;
using audiostream::AudioStream;

class AudioStreamClient {
public:
AudioStreamClient(std::shared_ptr<Channel> channel)
        : stub_(AudioStream::NewStub(channel)) {
}
void SaveAudio(std::string file_name, unsigned int rate) {
        AudioData data;
        ClientContext context;
        AudioRequest audio_req;
        audio_req.set_file_path(file_name);
        audio_req.set_stream_rate(rate);
        std::unique_ptr<ClientReader<AudioData> > reader (stub_->StreamAudio(&context, audio_req));
        std::ofstream file_client;
        file_client.open(file_name);
        auto t1 = std::chrono::system_clock::now();
        while (reader->Read(&data)) {
                file_client << data.audio_data();
        }
        auto t2 = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        std::cout << "Stream duration: " << duration.count() << "\n";
        file_client.close();
        Status status = reader->Finish();
        if(grpc::CANCELLED == status.error_code()) {
                auto a = std::remove(file_name.c_str());
                std::cout << "Error: " << file_name <<" not found!\n";
        } else {
                std::cout << "Successfully received file from server!\n";
        }
}
private:
std::unique_ptr<AudioStream::Stub> stub_;
};

int main(int argc, char** argv) {

        std::string server_details;
        std::string file_name;
        unsigned int rate;
        std::cout << "Create client request..." << std::endl;
        switch(argc) {
        case 1:
                server_details = "localhost:1337";
                file_name = "bible.txt";
                rate = 128;
                std::cout << "No file name or bit rate specified. Using default file and default bit rate of 128kBps.\n";
                break;
        case 2:
                std::cout << "Error!\nCorrect usage: " << argv[0] << "  <server_ip:port_number> <file_name> <stream_rate(unisgned int)> \n";
                return EXIT_FAILURE;
        case 3:
                std::cout << "Error!\nCorrect usage: " << argv[0] << "  <server_ip:port_number> <file_name> <stream_rate(unisgned int)> \n";
                return EXIT_FAILURE;
        case 4:
                server_details = argv[1];
                file_name = argv[2];
                rate = atoi(argv[3]);
                std::cout << "Requesting " << file_name << " from server at a stream rate of " << rate << "kB per streaming instance.\n";
                break;
        default:
                std::cout << "Error!\nCorrect usage: " << argv[0] << "  <file_name> <stream_rate(unisgned int)>\n";
                return EXIT_FAILURE;
        }
        AudioStreamClient stream_client(
                grpc::CreateChannel(server_details,
                                    grpc::InsecureChannelCredentials()));
        stream_client.SaveAudio(file_name, rate);

        return 0;
}
