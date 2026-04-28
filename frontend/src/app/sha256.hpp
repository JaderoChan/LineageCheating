#include <string>
#include <cstring>
#include <sstream>

class SHA256 {
private:
    uint32_t state[8];
    unsigned char buffer[64];
    uint64_t count;

    void transform(const unsigned char* data);
    void finalize(unsigned char* digest);

public:
    SHA256();
    void update(const unsigned char* data, size_t len);
    void update(const std::string& str);
    std::string hash();
};

// 构造函数：初始化哈希初始值
SHA256::SHA256() {
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
    count = 0;
}

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
#define CAP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define CAP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))

static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void SHA256::transform(const unsigned char* data) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = (data[i*4] << 24) | (data[i*4+1] << 16) | (data[i*4+2] << 8) | data[i*4+3];
    for (; i < 64; i++)
        w[i] = SIG1(w[i-2]) + w[i-7] + SIG0(w[i-15]) + w[i-16];

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (i = 0; i < 64; i++) {
        uint32_t t1 = h + CAP1(e) + CH(e,f,g) + k[i] + w[i];
        uint32_t t2 = CAP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void SHA256::update(const unsigned char* data, size_t len) {
    size_t i = 0;
    size_t bufpos = count % 64;
    count += len;

    if (bufpos > 0) {
        size_t copy = 64 - bufpos;
        if (copy > len) copy = len;
        memcpy(buffer + bufpos, data, copy);
        if (bufpos + copy == 64) transform(buffer);
        i += copy;
    }

    for (; i + 63 < len; i += 64)
        transform(data + i);

    if (i < len)
        memcpy(buffer, data + i, len - i);
}

void SHA256::update(const std::string& str) {
    update((const unsigned char*)str.c_str(), str.size());
}

void SHA256::finalize(unsigned char* digest) {
    uint64_t totalBits = count * 8;
    unsigned char pad[64] = {0};
    pad[0] = 0x80;
    size_t padlen = (count % 64 < 56) ? (56 - count%64) : (120 - count%64);
    update(pad, padlen);

    for (int i = 0; i < 8; i++)
        pad[i] = totalBits >> (56 - i*8);
    update(pad, 8);

    for (int i = 0; i < 8; i++) {
        digest[i*4] = state[i] >> 24;
        digest[i*4+1] = state[i] >> 16;
        digest[i*4+2] = state[i] >> 8;
        digest[i*4+3] = state[i];
    }
}

std::string SHA256::hash() {
    unsigned char digest[32];
    finalize(digest);
    std::stringstream ss;
    for (int i = 0; i < 32; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    return ss.str();
}

std::string sha256(const std::string& input) {
    SHA256 sha;
    sha.update(input);
    return sha.hash();
}
