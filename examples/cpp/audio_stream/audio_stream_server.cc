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
using audiostream::FilePath;
using audiostream::AudioData;
using audiostream::AudioStream;

class AudioStreamImpl final : public AudioStream::Service {
public:
explicit AudioStreamImpl() {
}

Status StreamAudio(ServerContext* context,
                   const audiostream::FilePath* audio,
                   ServerWriter<AudioData>* writer) override {
        auto audio_path = audio->file_path();
        input_file_.open(audio_path);
        if(!input_file_)
        {
                std::cout << "Could not open text file at the specified location." << std::endl;
                return Status::OK;
        }
        int next_index = 0;
        while (!input_file_.eof()) {
                AudioData d;
                char a_multi[131072]= {};
                input_file_.seekg(next_index*131072);
                input_file_.read(a_multi, 131072);
                std::cout << next_index << std::endl;
                // char a = input_file_.get();
                d.set_audio_data(&a_multi, 131072);
                writer->Write(d);
                next_index++;
        }
        input_file_.close();
        return Status::OK;
}
private:
std::ifstream input_file_;
};

void RunServer() {
        std::string server_address("192.168.0.199:1337");
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
