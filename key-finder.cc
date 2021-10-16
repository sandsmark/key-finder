#include <string>
#include <climits>
#include <cstring>
#include <cmath>

#define MIN_LENGTH 11
#define MAX_LENGTH 40

struct BucketMap {
    constexpr BucketMap() : map{}, revmap{} {
        // Clear out
        for (unsigned i=0; i<sizeof revmap; i++) {
            revmap[i] = 0;
        }

        // Fill up
        int num = 0;
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

        for (unsigned i='z' + 1; i<sizeof map; i++) {
            map[i] = 0;
        }
        bucketCount = ++num;
    }

    inline unsigned char operator[](const unsigned char index) const {
        return map[index];
    }

    unsigned char map[UCHAR_MAX];
    char revmap[UCHAR_MAX];
    int bucketCount = 0;
};
static const constexpr BucketMap mapping;

static double keyEntropy(const std::string &string)
{
    static unsigned buckets[mapping.bucketCount];
    memset(buckets, 0, sizeof buckets);

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

    return ent;
}

static double bigramEntropy(const std::string &string)
{
    static unsigned bigramBuckets[mapping.bucketCount][mapping.bucketCount];
    memset(bigramBuckets, 0, sizeof bigramBuckets);
    for (size_t i=0; i<string.size() - 1; i++) {
        const int pos = mapping[string[i]];
        const int nextPos = mapping[string[i+1]];
        bigramBuckets[pos][nextPos]++;
    }
    const double length = string.size();
    double bigramEnt = 0;
    for (size_t i=0; i<mapping.bucketCount; i++) {
        for (size_t j=0; j<mapping.bucketCount; j++) {
            if (!bigramBuckets[i][j]) {
                continue;
            }
            const double prob = bigramBuckets[i][j] / length;
            bigramEnt += prob * std::log2(1. / prob);
        }
    }
    return bigramEnt;
}

static double entropy(const std::string &string)
{
    static unsigned buckets[UCHAR_MAX];
    memset(buckets, 0, sizeof buckets);

    for (size_t i=0; i<string.size(); i++) {
        unsigned char c = string[i];
        buckets[c]++;
    }

    double ent = 0;
    const double length = string.size();
    for (size_t i=1; i<UCHAR_MAX; i++) {
        if (!buckets[i]) {
            continue;
        }
        const double prob = buckets[i] / length;
        ent += prob * std::log2(1. / prob);
    }

    return ent;
}

static double caseEntropy(const std::string &string, double *switches_)
{
    enum Type {
        Number,
        Upper,
        Lower,
        TypeCount
    };
    static unsigned buckets[TypeCount];
    memset(buckets, 0, sizeof buckets);

    int switches = 0;
    Type prevType = TypeCount;
    int longestRun = 0;
    int run = 0;
    for (size_t i=0; i<string.size(); i++) {
        const char c = string[i];
        Type type = TypeCount;
        if (c >= '0' && c<= '9') {
            type = Number;
        } else if (c >= 'a' && c <= 'z') {
            type = Lower;
        } else if (c >= 'A' && c <= 'Z') {
            type = Upper;
        }
        if (type != prevType) {
            switches++;
            prevType = type;
            longestRun = std::max(longestRun, run);
            run = 0;
        } else {
            run++;
        }
        buckets[type]++;
    }

    double ent = 0;
    const double length = string.size();
    for (size_t i=0; i<TypeCount; i++) {
        if (!buckets[i]) {
            continue;
        }
        const double prob = buckets[i] / length;
        ent += prob * std::log2(1. / prob);
    }
    const double swprob = switches ? switches / length : 0;
    if (swprob) {
        *switches_ = swprob * std::log2(1./swprob);
    } else {
        *switches_ = 0.;
    }
    //printf("\t\t%s %f %f\n", string.c_str(), ent, swprob ? swprob * std::log2(1./swprob) : 0);

    return ent;
}

static bool startsWith(const std::string &string, const char *needle)
{
    return strncmp(string.c_str(), needle, strlen(needle)) == 0;
}

void handleCandidate(const std::string &buffer)
{
    //if (startsWith(buffer, "AIza")) {
    //    puts(buffer.c_str());
    //    return;
    //}
    if (startsWith(buffer, "kExpr")) {
        return;
    }
    if (startsWith(buffer, "X64I64x") || startsWith(buffer, "X64I32x") || startsWith(buffer, "X64I16x") || startsWith(buffer, "X64S16x") || startsWith(buffer, "X64S8x16")) {
        return;
    }
    if (startsWith(buffer, "0123456789")) {
        return;
    }
    double switches;
    const double cent = caseEntropy(buffer, &switches);
    const double kent = keyEntropy(buffer);
    //printf("\t%s:\t%f\t%f\t%f\t%f\n", buffer.c_str(), entropy(buffer), keyEntropy(buffer), bigramEntropy(buffer), caseEntropy(buffer, &switches));
        //printf("\t%s: %f %f %f\n", buffer.c_str(), cent, kent, switches);
    if (startsWith(buffer, "AIza") || (cent > 1.28 && kent > 3.8 && switches < 0.45)) {
        printf("%s: %f %f %f\n", buffer.c_str(), cent, kent, switches);
        //printf("%s\n", buffer.c_str());
    }

}

void readFile(FILE *file)
{
    std::string buffer;
    buffer.reserve(MAX_LENGTH);
    int ret;
    size_t pos = 0;
    while ((ret = getc(file)) != EOF) {
        const unsigned char c = ret;
        //if (++pos % 1024 == 0) {
        //    printf(".");
        //    fflush(stdout);
        //}
        if (!mapping[c]) {
            if (buffer.size() > MIN_LENGTH && buffer.size() < MAX_LENGTH) {
                handleCandidate(buffer);
            }
            buffer.clear();
            continue;
        }
        if (buffer.size() >= MAX_LENGTH) {
            continue;
        }
        buffer.push_back(c);
    }
    printf("%ld bytes read\n", ftell(file));

    puts("Done.");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        puts("Pass a file");
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open input");
        return 1;
    }

    readFile(file);

    fclose(file);

    return 0;
}
