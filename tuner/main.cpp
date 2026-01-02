#include "tuner.h"

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
        solo::tuner::run(eval::Weight());
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

        solo::tuner::run(w, init_id);
    }

    return 0;
};