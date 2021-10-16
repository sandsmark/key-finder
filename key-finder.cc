#include <string>
#include <climits>
#include <cstring>
#include <cmath>

struct BucketMap {
    constexpr BucketMap() : map{}, revmap{} {
        int num = 0;

        for (int i=0; i<sizeof revmap; i++) {
            revmap[i] = 0;
        }

        for (int i=0; i<'-'; i++) {
            map[i] = 0;
        }

        map['-'] = ++num;
        revmap[num] = '-';

        for (int i='-' + 1; i<'0'; i++) {
            map[i] = 0;
        }

        for (int i='0'; i<='9'; i++) {
            map[i] = ++num;
            revmap[num] = i;
        }

        for (int i='9' + 1; i<'A'; i++) {
            map[i] = 0;
        }

        for (int i='A'; i<='Z'; i++) {
            map[i] = ++num;
            revmap[num] = i;
        }

        for (int i='Z' + 1; i<'a'; i++) {
            map[i] = 0;
        }

        for (int i='a'; i<='z'; i++) {
            map[i] = ++num;
            revmap[num] = i;
        }

        for (int i='z' + 1; i<sizeof map; i++) {
            map[i] = 0;
        }
        bucketCount = ++num;
    }

    inline char operator[](const unsigned char index) const {
        return map[index];
    }

    char map[UCHAR_MAX];
    char revmap[UCHAR_MAX];
    int bucketCount = 0;
};

double entropy(const std::string &string)
{
    static const constexpr BucketMap mapping;
    for (int i=0; i<sizeof mapping.map; i++) {
        printf("%c (%d): %d\n", (i >= ' ' && i < 127) ? char(i): ' ', i, mapping[i]);
    }
    printf("%d\n", mapping.bucketCount);

    static unsigned buckets[mapping.bucketCount];
    memset(buckets, sizeof buckets, 0);
    for (size_t i=0; i<string.size(); i++) {
        buckets[mapping[string[i]]]++;
    }

    double ent = 0;
    const double length = string.size();
    for (size_t i=1; i<mapping.bucketCount; i++) {
        if (!buckets[i]) {
            continue;
        }
        const double prob = buckets[i] / length;
        ent += prob * std::log2(1. / prob);
    }

    printf("%f\n", ent);

    return ent;
}

int main(int argc, char *argv[])
{
    entropy("foo");
    return 0;
}
