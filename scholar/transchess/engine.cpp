// engine.cpp
// need a passable chess engine
// Shaun Harker 2022-05-01
// MIT LICENSE

#include <array>
#include <vector>
#include <cstdint>
#include <iostream>
#include <functional>


typedef std::function<uint64_t(uint64_t)> Fun;
typedef std::array<Fun,2> FunList2;
typedef std::array<Fun,4> FunList4;
typedef std::array<Fun,8> FunList8;
typedef std::tuple<uint64_t, uint64_t, uint64_t> Move;
typedef std::vector<Move> MoveList;


// move flags
constexpr uint64_t standard = uint64_t(1) << 1;
constexpr uint64_t pawnpush = uint64_t(1) << 2;
constexpr uint64_t castleQ = uint64_t(1) << 3;
constexpr uint64_t castleK = uint64_t(1) << 4;
constexpr uint64_t enpassant = uint64_t(1) << 5;
constexpr uint64_t promoteQ = uint64_t(1) << 8;
constexpr uint64_t promoteR = uint64_t(1) << 9;
constexpr uint64_t promoteB = uint64_t(1) << 10;
constexpr uint64_t promoteN = uint64_t(1) << 11;
constexpr uint64_t promote = promoteQ | promoteR | promoteB | promoteN;

// double pawn pushes: the move flag will be the square in front of pawn before moving


// ((a8, b8, c8, d8, e8, f8, g8, h8),
//  (a7, b7, c7, d7, e7, f7, g7, h7),
//  (a6, b6, c6, d6, e6, f6, g6, h6),
//  (a5, b5, c5, d5, e5, f5, g5, h5),
//  (a4, b4, c4, d4, e4, f4, g4, h4),
//  (a3, b3, c3, d3, e3, f3, g3, h3),
//  (a2, b2, c2, d2, e2, f2, g2, h2),
//  (a1, b1, c1, d1, e1, f1, g1, h1)) = (
//     [[ uint64_t64(1) << (i+8*j) for i in range(8)] for j in range(8)])
//


constexpr uint64_t a8 = uint64_t(1) << 0;
constexpr uint64_t b8 = uint64_t(1) << 1;
constexpr uint64_t c8 = uint64_t(1) << 2;
constexpr uint64_t d8 = uint64_t(1) << 3;
constexpr uint64_t e8 = uint64_t(1) << 4;
constexpr uint64_t f8 = uint64_t(1) << 5;
constexpr uint64_t g8 = uint64_t(1) << 6;
constexpr uint64_t h8 = uint64_t(1) << 7;

constexpr uint64_t a7 = uint64_t(1) << 8;
constexpr uint64_t b7 = uint64_t(1) << 9;
constexpr uint64_t c7 = uint64_t(1) << 10;
constexpr uint64_t d7 = uint64_t(1) << 11;
constexpr uint64_t e7 = uint64_t(1) << 12;
constexpr uint64_t f7 = uint64_t(1) << 13;
constexpr uint64_t g7 = uint64_t(1) << 14;
constexpr uint64_t h7 = uint64_t(1) << 15;

constexpr uint64_t a6 = uint64_t(1) << 16;
constexpr uint64_t b6 = uint64_t(1) << 17;
constexpr uint64_t c6 = uint64_t(1) << 18;
constexpr uint64_t d6 = uint64_t(1) << 19;
constexpr uint64_t e6 = uint64_t(1) << 20;
constexpr uint64_t f6 = uint64_t(1) << 21;
constexpr uint64_t g6 = uint64_t(1) << 22;
constexpr uint64_t h6 = uint64_t(1) << 23;

constexpr uint64_t a5 = uint64_t(1) << 24;
constexpr uint64_t b5 = uint64_t(1) << 25;
constexpr uint64_t c5 = uint64_t(1) << 26;
constexpr uint64_t d5 = uint64_t(1) << 27;
constexpr uint64_t e5 = uint64_t(1) << 28;
constexpr uint64_t f5 = uint64_t(1) << 29;
constexpr uint64_t g5 = uint64_t(1) << 30;
constexpr uint64_t h5 = uint64_t(1) << 31;

constexpr uint64_t a4 = uint64_t(1) << 32;
constexpr uint64_t b4 = uint64_t(1) << 33;
constexpr uint64_t c4 = uint64_t(1) << 34;
constexpr uint64_t d4 = uint64_t(1) << 35;
constexpr uint64_t e4 = uint64_t(1) << 36;
constexpr uint64_t f4 = uint64_t(1) << 37;
constexpr uint64_t g4 = uint64_t(1) << 38;
constexpr uint64_t h4 = uint64_t(1) << 39;

constexpr uint64_t a3 = uint64_t(1) << 40;
constexpr uint64_t b3 = uint64_t(1) << 41;
constexpr uint64_t c3 = uint64_t(1) << 42;
constexpr uint64_t d3 = uint64_t(1) << 43;
constexpr uint64_t e3 = uint64_t(1) << 44;
constexpr uint64_t f3 = uint64_t(1) << 45;
constexpr uint64_t g3 = uint64_t(1) << 46;
constexpr uint64_t h3 = uint64_t(1) << 47;

constexpr uint64_t a2 = uint64_t(1) << 48;
constexpr uint64_t b2 = uint64_t(1) << 49;
constexpr uint64_t c2 = uint64_t(1) << 50;
constexpr uint64_t d2 = uint64_t(1) << 51;
constexpr uint64_t e2 = uint64_t(1) << 52;
constexpr uint64_t f2 = uint64_t(1) << 53;
constexpr uint64_t g2 = uint64_t(1) << 54;
constexpr uint64_t h2 = uint64_t(1) << 55;

constexpr uint64_t a1 = uint64_t(1) << 56;
constexpr uint64_t b1 = uint64_t(1) << 57;
constexpr uint64_t c1 = uint64_t(1) << 58;
constexpr uint64_t d1 = uint64_t(1) << 59;
constexpr uint64_t e1 = uint64_t(1) << 60;
constexpr uint64_t f1 = uint64_t(1) << 61;
constexpr uint64_t g1 = uint64_t(1) << 62;
constexpr uint64_t h1 = uint64_t(1) << 63;

constexpr uint64_t file_a = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
constexpr uint64_t file_b = b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8;
constexpr uint64_t file_c = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
constexpr uint64_t file_d = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
constexpr uint64_t file_e = e1 + e2 + e3 + e4 + e5 + e6 + e7 + e8;
constexpr uint64_t file_f = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
constexpr uint64_t file_g = g1 + g2 + g3 + g4 + g5 + g6 + g7 + g8;
constexpr uint64_t file_h = h1 + h2 + h3 + h4 + h5 + h6 + h7 + h8;

constexpr uint64_t rank_8 = a8 + b8 + c8 + d8 + e8 + f8 + g8 + h8;
constexpr uint64_t rank_7 = a7 + b7 + c7 + d7 + e7 + f7 + g7 + h7;
constexpr uint64_t rank_6 = a6 + b6 + c6 + d6 + e6 + f6 + g6 + h6;
constexpr uint64_t rank_5 = a5 + b5 + c5 + d5 + e5 + f5 + g5 + h5;
constexpr uint64_t rank_4 = a4 + b4 + c4 + d4 + e4 + f4 + g4 + h4;
constexpr uint64_t rank_3 = a3 + b3 + c3 + d3 + e3 + f3 + g3 + h3;
constexpr uint64_t rank_2 = a2 + b2 + c2 + d2 + e2 + f2 + g2 + h2;
constexpr uint64_t rank_1 = a1 + b1 + c1 + d1 + e1 + f1 + g1 + h1;

auto w(uint64_t x) -> uint64_t {return (x << 1) & ~(file_a);}
auto e(uint64_t x) -> uint64_t {return (x >> 1) & ~(file_h);}
auto s(uint64_t x) -> uint64_t {return (x << 8) & ~(rank_8);}
auto n(uint64_t x) -> uint64_t {return (x >> 8) & ~(rank_1);}
auto nw(uint64_t x) -> uint64_t {return n(w(x));}
auto ne(uint64_t x) -> uint64_t {return n(e(x));}
auto sw(uint64_t x) -> uint64_t {return s(w(x));}
auto se(uint64_t x) -> uint64_t {return s(e(x));}
auto nwn(uint64_t x) -> uint64_t {return n(w(n(x)));}
auto nen(uint64_t x) -> uint64_t {return n(e(n(x)));}
auto sws(uint64_t x) -> uint64_t {return s(w(s(x)));}
auto ses(uint64_t x) -> uint64_t {return s(e(s(x)));}
auto wnw(uint64_t x) -> uint64_t {return w(n(w(x)));}
auto ene(uint64_t x) -> uint64_t {return e(n(e(x)));}
auto wsw(uint64_t x) -> uint64_t {return w(s(w(x)));}
auto ese(uint64_t x) -> uint64_t {return e(s(e(x)));}

const FunList8 kingmoves {n, w, s, e, nw, ne, sw, se};
const FunList4 bishopmoves {nw, ne, sw, se};
const FunList4 rookmoves {n, w, s, e};
const FunList8 knightmoves {nwn, nen, wsw, wnw, sws, ses, ese, ene};
const FunList2 whitepawncaptures {nw, ne};
const FunList2 blackpawncaptures {sw, se};

template <typename Fs>
auto hopper(uint64_t x, Fs S) -> uint64_t {
  uint64_t bb = 0;
  for (auto it = S.begin(); it != S.end(); ++ it) {
    auto f = *it;
    bb |= f(x);
  }
  return bb;
}

template <typename F>
auto ray(uint64_t x, F f, uint64_t empty) -> uint64_t {
  uint64_t bb = 0;
  auto y = f(x);
  while (y & empty) {
    bb |= y;
    y = f(y);
  }
  bb |= y;
  return bb;
}

template <typename Fs>
auto slider(uint64_t x, Fs S, uint64_t empty) -> uint64_t {
  uint64_t bb = 0;
  for (auto it = S.begin(); it != S.end(); ++ it) {
    auto f = *it;
    bb |= ray(x, f, empty);
  }
  return bb;
}

template <typename F>
void bitapply(uint64_t x, F f) {
  uint64_t tmp, lsb;
  while (x) {
    tmp = x & (x-1);
    lsb = x ^ tmp;
    f(lsb);
    x = tmp;
  }
}

class Engine {
public:
  uint64_t white;
  uint64_t black;
  uint64_t king;
  uint64_t queen;
  uint64_t rook;
  uint64_t bishop;
  uint64_t knight;
  uint64_t pawn;
  uint64_t ply;
  uint64_t castling;
  uint64_t enpassant;
  uint64_t halfmove;
  uint64_t fullmove;

  Engine(){
    white = rank_1 | rank_2;
    black = rank_7 | rank_8;
    king = e1 | e8;
    queen = d1 | d8;
    rook = a1 | a8 | h1 | h8;
    bishop = c1 | c8 | f1 | f8;
    knight = b1 | b8 | g1 | g8;
    pawn = rank_2 | rank_7;
    ply = 0;
    castling = a1 | a8 | h1 | h8;
    enpassant = 0;
    halfmove = 0;
    fullmove = 1;
  }

  uint64_t threat(bool white_to_move) const {
    // A square is threatened if a pseudolegal move from the
    // opposing side could target it were it unoccupied.
    uint64_t occupied = white | black;
    uint64_t empty = ~occupied;
    uint64_t bb = 0;
    uint64_t us = white_to_move ? white : black;

    bitapply(us & king, [&](uint64_t x) {
      bb |= hopper(x, kingmoves);
    });

    bitapply(us & (queen | rook), [&](uint64_t x) {
        bb |= slider(x, rookmoves, empty);
    });

    bitapply(us & (queen | bishop), [&](uint64_t x) {
        bb |= slider(x, bishopmoves, empty);
    });

    bitapply(us & knight, [&](uint64_t x) {
        bb |= hopper(x, knightmoves);
    });

    bitapply(us & pawn, [&](uint64_t x) {
      bb |= hopper(x, white_to_move ? whitepawncaptures : blackpawncaptures);
    });

    return bb;
  }

  MoveList pseudolegal() const {
    // Remove the "cannot move into check" rule from the game,
    // and the resulting moves are called pseudolegal moves.
    //
    // However we distinguish this from "cannot castle through check"
    // which we regard as an independent rule, as it is based on the
    // enemy threat before the move rather than after the move.
    //
    // Therefore, it is pseudolegal to castle into check as long
    // as it is not *through* check.

    MoveList moves;
    bool white_to_move = (ply % 2 == 0);
    uint64_t us = white_to_move ? white : black;
    uint64_t them = white_to_move ? black : white;
    uint64_t backrank = white_to_move ? rank_1 : rank_8;
    uint64_t endrank = white_to_move ? rank_8 : rank_1;
    uint64_t notendrank = white_to_move ? ~rank_8 : ~rank_1;
    uint64_t doublepawnpush = white_to_move ? rank_4 : rank_5;
    uint64_t occupied = white | black;
    uint64_t empty = ~occupied;
    uint64_t safe = ~threat(~white_to_move);
    uint64_t safe_and_empty = safe & empty; // the square we pass during castling must be in safe_and_empty
    uint64_t empty_or_them = empty | them;

    auto add = [&](uint64_t source, uint64_t targets, uint64_t flags) {
      bitapply(targets, [&](uint64_t target){
        bitapply(flags, [&](uint64_t flag) {
          moves.push_back({source, target, flag});
        });
      });
    };

    bitapply(us & king, [&](uint64_t x){
      auto Y = hopper(x, kingmoves);
      add(x, Y & empty_or_them, standard);
      uint64_t castlerooks = us & rook & castling;
      add(x, e(e(e(castlerooks))) & e(e(empty)) & e(empty) & empty & w(safe_and_empty) & w(w(x)), castleQ);
      add(x, e(e(x)) & e(safe_and_empty) & empty & w(empty) & w(w(castlerooks)), castleK);
    });

    bitapply(us & (queen | rook), [&](uint64_t x){
      auto Y = slider(x, rookmoves, empty);
      add(x, Y & empty_or_them, standard);
    });

    bitapply(us & (queen | bishop), [&](uint64_t x){
      auto Y = slider(x, bishopmoves, empty);
      add(x, Y & empty_or_them, standard);
    });

    bitapply(us & knight, [&](uint64_t x){
      auto Y = hopper(x, knightmoves);
      add(x, Y & empty_or_them, standard);
    });

    bitapply(us & pawn, [&](uint64_t x){
      auto f = white_to_move ? n : s;
      add(x, f(f(x)) & f(empty) & empty & doublepawnpush, f(x));
      add(x, f(x) & empty & notendrank, pawnpush);
      add(x, f(x) & empty & endrank, promote);
      auto S = white_to_move ? whitepawncaptures : blackpawncaptures;
      auto Y = hopper(x, S);
      add(x, Y & enpassant, enpassant);
      add(x, Y & them & notendrank, standard);
      add(x, Y & them & endrank, promote);
    });

    return moves;
  }

  void move(Move m) {
    bool white_to_move = (ply % 2 == 0);
    uint64_t us = white_to_move ? white : black;
    uint64_t them = white_to_move ? black : white;
    uint64_t backrank = white_to_move ? rank_1 : rank_8;
    uint64_t endrank = white_to_move ? rank_8 : rank_1;
    const auto& [source, target, flag] = m;

    // handle target square
    // 1. if empty, no problem
    // 2. if piece, remove it from its bitboard
    // 3. remove from `them`
    // 4. if an enpassant target, capture en passant
    uint64_t mask = ~target;
    them &= mask;
    queen &= mask;
    rook &= mask;
    bishop &= mask;
    knight &= mask;
    pawn &= mask;
    if (flag == enpassant) {
      // en passant capture. "notsofast" is uint64_t(1) << i,
      // where i in {0, ..., 63} is the square the
      // pawn being captured occupies.
      uint64_t notsofast = (white_to_move ? s : n)(target);
      pawn ^= notsofast;
      them ^= notsofast;
    }

    // handle source square
    // 1. move piece over to target square
    // 2. update castling rights if needed
    uint64_t mover = source | target;
    us ^= mover;
    if (source & king) {
      king ^= mover;
      if (white_to_move) {
        castling &= ~(a1 | h1);
      } else {
        castling &= ~(a8 | h8);
      }
    } else if (source & queen) {
      queen ^= mover;
    } else if (source & rook) {
      rook ^= mover;
      castling &= ~source;
    } else if (source & bishop) {
      bishop ^= mover;
    } else if (source & knight) {
      knight ^= mover;
    } else if (source & pawn) {
      pawn ^= mover;
    }

    // handle promotions
    uint64_t promoted = pawn & endrank;
    pawn ^= promoted;
    if (promoted) {
      switch(flag) {
        case promoteQ:
          queen ^= promoted;
        case promoteR:
          rook ^= promoted;
        case promoteB:
          bishop ^= promoted;
        case promoteN:
          knight ^= promoted;
      }
    }

    // handle castling rooks
    if (flag == castleK) {
        uint64_t rook_mover = e(target) | w(target);
        rook ^= rook_mover;
        us ^= rook_mover;
    }

    if (flag == castleQ) {
        uint64_t rook_mover = w(w(target)) | e(target);
        rook ^= rook_mover;
        us ^= rook_mover;
    }

    // handle en passant flags
    if (flag & (white_to_move ? rank_3 : rank_6)) {
        enpassant = flag;
    } else {
        enpassant = 0;
    }

    // handle ply
    ply += 1;

    // handle half-move clock
    halfmove += 1;
    if ((target & them != 0) or
        (flag == enpassant) or
        ((flag & (rank_3 | rank_6 | pawnpush)) != 0)) {
      halfmove = 0;
    }

    // handle full-move number
    if (!white_to_move) {
        fullmove += 1;
    }
  }

  MoveList legal_moves() const {
    // A move is legal if:
    //   1. It is pseudolegal
    //   2. If performed, then there isn't a pseudolegal king capture
    bool white_to_move = (ply % 2 == 0);
    auto us = white_to_move ? white : black;
    auto them = white_to_move ? black : white;
    auto backrank = white_to_move ? rank_1 : rank_8;
    auto endrank = white_to_move ? rank_8 : rank_1;

    //check = us & king & threat(~white_to_move);

    MoveList moves;
    MoveList pl_moves = pseudolegal();
    for (auto it = pl_moves.begin(); it != pl_moves.end(); ++ it) {
      auto move = *it;
      // see if move puts us in check
      Engine after(*this); // fork a new position
      after.move(move);
      uint64_t threat_after = after.threat(~white_to_move);
      uint64_t us_after = white_to_move ? after.white : after.black;
      uint64_t check_after = us_after & after.king & threat_after;
      if (check_after == 0) moves.push_back(move);
    }
    return moves;
  }
};


int ntz(uint64_t x) {
    // Return the number of trailing zeros in
    // the binary representation of x.
    // Assumes 0 <= x < 2**64.
    constexpr uint64_t debruijn = 0x03f79d71b4cb0a89;
    constexpr std::array<int,64> lookup =
        {0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63};
    return lookup[(((debruijn*(x^(x-1))) >> 58) + 64)%64];
}

std::string square(int s) {
  char cstr[3] {(char)('a' + s%8), (char)('8' - s/8), 0};
  return std::string(cstr);
}

int main(int argc, char * argv []) {
  Engine e;
  MoveList moves = e.legal_moves();
  for (auto it=moves.begin(); it!=moves.end(); ++it) {
    auto move = *it;
    const auto& [source, target, flag] = move;
    int s = ntz(source);
    int t = ntz(target);
    std::cout << square(s) << square(t) << "\n";
  }
  return 0;
}
