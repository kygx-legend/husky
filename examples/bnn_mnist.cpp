// Copyright 2017 Husky Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <string>
#include <vector>

#include "boost/tokenizer.hpp"

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"
#include "lib/vector.hpp"

namespace husky {

class Image {
   public:
    using KeyT = int;

    Image() = default;
    explicit Image(KeyT k) : key(k) {}
    Image(KeyT k, const lib::SparseVectorXd& f, int l) {
        key = k;
        feature = f;
        label = l;
    }

    const KeyT& id() const { return key; }

    KeyT key;
    lib::SparseVectorXd feature;
    int label;

    friend BinStream& operator<< (BinStream& stream, const Image& image) {
        return stream << image.key << image.feature << image.label;
    }

    friend BinStream& operator>> (BinStream& stream, Image& image) {
        return stream >> image.key >> image.feature >> image.label;
    }
};

static int s_count = 0;

void bnn() {
    auto& infmt = husky::io::InputFormatStore::create_line_inputformat();
    infmt.set_input(husky::Context::get_param("train_set"));

    auto& images_list = husky::ObjListStore::create_objlist<Image>();

    auto parse = [&](boost::string_ref& chunk) {
        if (chunk.size() == 0)
            return;
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char>> tok(chunk, sep);
        int index = 0;
        Image img(s_count++);
        img.feature.resize(28 * 28);
        for (auto& w : tok) {
            if (index == 0) {
               img.label = std::stoi(w);
               index++;
               continue;
            }
            int v = std::stoi(w);
            if (v != 0)
                img.feature.coeffRef(index - 1) = double(v) / double(255);
            index++;
        }
        LOG_I << img.label << ":" << index;
        //LOG_I << img.feature;
    };

    husky::load(infmt, parse);

    LOG_I << "total: " << s_count;
}

}  // namespace husky

int main(int argc, char** argv) {
    if (husky::init_with_args(argc, argv)) {
        husky::run_job(husky::bnn);
        return 0;
    }
    return 1;
}
