#include "tuner.h"
#include "battle.h"

int main()
{
    srand(time(NULL));

    i32 user;

    printf("Choose an action:\n");
    printf("[0] - Train default weights\n");
    printf("[1] - Train custom weights\n");
    printf("[2] - Print value\n");

    std::cin >> user;

    if (user == 0) {
        tuner::run(eval::Weight());
    }
    else if (user == 1) {
        auto w = eval::Weight();
        std::ifstream f("config.json");
        if (!f.good()) {
            printf("Can't find \"config.json\"!\n");
            return -1;
        };
        json js;
        f >> js;
        f.close();
        from_json(js, w);

        i32 init_id;

        std::cin >> init_id;

        tuner::run(w, init_id);
    }
    else if (user == 2) {
        i32 idx = 0;

        std::string out_str;
        std::string out_id_str;

        while (true)
        {
            std::string id = std::to_string(idx);
            std::string fname = std::string("data/") + id + std::string(".json");

            std::ifstream f(fname);
            if (!f.good()) {
                break;
            };

            tuner::SaveData s;
            tuner::load(id, s);

            out_str += std::to_string(s.result.attack) + "\n";
            out_id_str += id + "\n";

            idx += 1;
        }

        std::ofstream o("out.txt");
        o << out_str << std::endl;
        o << out_id_str << std::endl;
        o.close();
    }

    return 0;
};