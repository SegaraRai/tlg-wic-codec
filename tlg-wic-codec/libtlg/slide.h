//---------------------------------------------------------------------------
#ifndef SLIDE_H
#  define SLIDE_H

#  include <array>

//---------------------------------------------------------------------------
constexpr int SLIDE_N = 4096;
constexpr int SLIDE_M = 18 + 255;

class SlideCompressor {
  template<typename T, std::size_t S>
  static constexpr std::array<T, S> CreateArray(T value) {
    std::array<T, S> arr{};
    // NOTE: std::array::fill is not constexpr in C++17
    for (std::size_t i = 0; i < S; i++) {
      arr[i] = value;
    }
    return arr;
  }

  // スライド辞書法 圧縮クラス
  struct Chain {
    int Prev;
    int Next;
  };

  static constexpr auto InitialText = CreateArray<unsigned char, SLIDE_N + SLIDE_M - 1>(0);
  static constexpr auto InitialMap = CreateArray<int, 256 * 256>(-1);
  static constexpr auto InitialChains = CreateArray<Chain, SLIDE_N>({ -1, -1 });

  std::array<unsigned char, SLIDE_N + SLIDE_M - 1> Text = InitialText;
  std::array<int, 256 * 256> Map = InitialMap;
  std::array<Chain, SLIDE_N> Chains = InitialChains;

  std::array<unsigned char, SLIDE_N + SLIDE_M - 1> Text2 = InitialText;
  std::array<int, 256 * 256> Map2 = InitialMap;
  std::array<Chain, SLIDE_N> Chains2 = InitialChains;

  int S = 0;
  int S2 = 0;

public:
  SlideCompressor();
  virtual ~SlideCompressor();

private:
  int GetMatch(const unsigned char* cur, int curlen, int& pos, int s);
  void AddMap(int p);
  void DeleteMap(int p);

public:
  void Encode(const unsigned char* in, long inlen, unsigned char* out, long& outlen);

  void Store();
  void Restore();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
