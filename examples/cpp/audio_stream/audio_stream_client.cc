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
using audiostream::FilePath;
using audiostream::AudioData;
using audiostream::AudioStream;

class AudioStreamClient {
public:
AudioStreamClient(std::shared_ptr<Channel> channel)
        : stub_(AudioStream::NewStub(channel)) {
}
void SaveAudio(std::string file_name) {
        AudioData data;
        ClientContext context;
        FilePath file_path;
        file_path.set_file_path(file_name);
        std::ofstream file_client;
        file_client.open("client_file.txt");

        std::unique_ptr<ClientReader<AudioData> > reader (stub_->StreamAudio(&context, file_path));
        while (reader->Read(&data)) {
                file_client << data.audio_data();
        }
        file_client.close();
        Status status = reader->Finish();
}
private:
std::unique_ptr<AudioStream::Stub> stub_;
};

int main(int argc, char** argv) {
        AudioStreamClient stream_client(
                grpc::CreateChannel("192.168.0.130:1337",
                                    grpc::InsecureChannelCredentials()));

        std::cout << "-------------- Get Audio stream file --------------" << std::endl;
        std::string file_name = "/Users/mukunda/Downloads/large/bible.txt";
        std::cout << "-------------- Saving Audio stream --------------" << std::endl;
        stream_client.SaveAudio(file_name);

        return 0;
}
