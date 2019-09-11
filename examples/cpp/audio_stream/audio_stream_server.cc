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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#ifdef BAZEL_BUILD
#include "examples/protos/audio_stream.grpc.pb.h"
#else
#include "audio_stream.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using audiostream::AudioRequest;
using audiostream::AudioData;
using audiostream::AudioStream;

class AudioStreamImpl final : public AudioStream::Service {
public:
explicit AudioStreamImpl() {
}

Status StreamAudio(ServerContext* context,
                   const audiostream::AudioRequest* audio_req,
                   ServerWriter<AudioData>* writer) override {
        constexpr unsigned int kilobyte = 1024;
        auto audio_path = audio_req->file_path();
        const unsigned int rate = audio_req->stream_rate();
        std::stringstream full_file_path;
        std::string home_path = std::getenv("HOME");
        full_file_path << home_path << "/grpc-test-files/" << audio_path;
        input_file_.open(full_file_path.str(), std::ios::ate);
        auto file_size = input_file_.tellg();
        if(!input_file_)
        {
                return Status::CANCELLED;
        }
        unsigned int buf_size = rate*kilobyte;
        unsigned int num_complete_blocks = file_size/buf_size;
        unsigned int remainder_block = file_size % buf_size;
        for (int idx = 0; idx < num_complete_blocks; idx++) {
                AudioData d;
                std::unique_ptr<char[]> data_array(new char[buf_size]());
                input_file_.seekg(idx*buf_size);
                input_file_.read(data_array.get(), buf_size);
                d.set_audio_data(data_array.get(), buf_size);
                writer->Write(d);
        }
        if (remainder_block != 0) {
                AudioData d;
                std::unique_ptr<char[]> data_array(new char[remainder_block]());
                input_file_.seekg(num_complete_blocks*remainder_block);
                input_file_.read(data_array.get(), remainder_block);
                d.set_audio_data(data_array.get(), remainder_block);
                writer->Write(d);
        }
        input_file_.close();
        return Status::OK;
}
private:
std::ifstream input_file_;
};

void RunServer() {
        std::string server_address("localhost:1337");
        AudioStreamImpl streamer;

        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&streamer);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
}

int main(int argc, char** argv) {
        // Expect only arg: --db_path=path/to/route_guide_db.json.
        RunServer();

        return 0;
}
