#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

enum class Piece
{
    Pawn = 0,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None
};

enum class Color
{
    White = 0,
    Black,
    Empty
};

class Tile
{
public:
    Tile(Color color_, Piece piece_, bool oob_ = false)
        : color(color_), piece(piece_), oob(oob_)
    {}

    const Color color;
    const Piece piece;
    const bool oob; // Out of bounds
};

Tile char_to_tile(char c)
{
    switch (c)
    {
        case 'p': return Tile{Color::Black, Piece::Pawn};
        case 'P': return Tile{Color::White, Piece::Pawn};
        case 'n': return Tile{Color::Black, Piece::Knight};
        case 'N': return Tile{Color::White, Piece::Knight};
        case 'b': return Tile{Color::Black, Piece::Bishop};
        case 'B': return Tile{Color::White, Piece::Bishop};
        case 'r': return Tile{Color::Black, Piece::Rook};
        case 'R': return Tile{Color::White, Piece::Rook};
        case 'q': return Tile{Color::Black, Piece::Queen};
        case 'Q': return Tile{Color::White, Piece::Queen};
        case 'k': return Tile{Color::Black, Piece::King};
        case 'K': return Tile{Color::White, Piece::King};
        default: std::cout << "unhandled char in char_to_tile" << std::endl; return Tile{Color::Empty, Piece::None};
    }
}

char tile_to_char(Tile t)
{
    if (t.color == Color::Black)
    {
        switch (t.piece)
        {
            case Piece::Pawn:   return 'p';
            case Piece::Knight: return 'n';
            case Piece::Bishop: return 'b';
            case Piece::Rook:   return 'r';
            case Piece::Queen:  return 'q';
            case Piece::King:   return 'k';
            case Piece::None:   return '.';
            default:            return '?';
        }
    }
    else
    {
        switch (t.piece)
        {
            case Piece::Pawn:   return 'P';
            case Piece::Knight: return 'N';
            case Piece::Bishop: return 'B';
            case Piece::Rook:   return 'R';
            case Piece::Queen:  return 'Q';
            case Piece::King:   return 'K';
            case Piece::None:   return '.';
            default:            return '?';
        }
    }
}

class Bitboard
{
public:
    bool read(std::uint8_t x, std::uint8_t y) const
    {
        return board & (std::uint64_t{1} << (x + y*8));
    }

    void write(std::uint8_t x, std::uint8_t y, bool value)
    {
        //board ^= (-value ^ board) & (1 << (x + y*8));
        if (value)
            board |= (std::uint64_t{1} << (y*8+x));
        else
            board &= (~(std::uint64_t{1} << (y*8+x)));
    }

    std::uint8_t count() const
    {
        std::uint8_t c = 0;

        for (std::uint8_t i = 0; i < 64; i++)
        {
            if (board & (std::uint64_t{1} << i))
                c++;
        }

        return c;
    }

    void print() const
    {
        std::string s(8*9, ' ');

        for (std::uint8_t i = 0; i < 8; i++)
        {
            s.at(i*9+8) = '\n';
        }

        for (std::uint8_t x = 0; x < 8; x++)
        {
            for (std::uint8_t y = 0; y < 8; y++)
            {
                std::uint8_t pos = (7-y)*9+x;

                s.at(pos) = '0' + static_cast<std::uint8_t>(read(x,y));
                //std::cout << std::to_string(x) << ' ' << std::to_string(y) << " = " << std::to_string(static_cast<std::uint8_t>(read(x,y))) << std::endl;
            }
        }

        std::cout << s;
    }

    std::uint64_t board = 0;
};

class Move
{
public:
    Move() = delete;

    // From x, from y, to x, to y
    Move(std::uint8_t fx_, std::uint8_t fy_, std::uint8_t tx_, std::uint8_t ty_, Piece promo_ = Piece::None)
        : fx(fx_), fy(fy_), tx(tx_), ty(ty_), promo(promo_)
    {}

    Move(std::string s)
    {
        fx = static_cast<std::uint8_t>(s.at(0)-'a');
        fy = static_cast<std::uint8_t>(s.at(1)-'1');
        tx = static_cast<std::uint8_t>(s.at(2)-'a');
        ty = static_cast<std::uint8_t>(s.at(3)-'1');

        if (s.size() == 5)
        {
            switch (s.at(4))
            {
                case 'n': promo = Piece::Knight; break;
                case 'b': promo = Piece::Bishop; break;
                case 'r': promo = Piece::Rook;   break;
                case 'q': promo = Piece::Queen;  break;
                default: break;
            }
        }
    }

    Move(const Move &m)
        : fx(m.fx), fy(m.fy), tx(m.tx), ty(m.ty), promo(m.promo)
    {}

    std::string longform() const
    {
        std::string s;

        s += static_cast<char>('a'+fx);
        s += static_cast<char>('1'+fy);
        s += static_cast<char>('a'+tx);
        s += static_cast<char>('1'+ty);

        switch (promo)
        {
            case Piece::None: break;
            case Piece::Knight: s += 'n'; break;
            case Piece::Bishop: s += 'b'; break;
            case Piece::Rook:   s += 'r'; break;
            case Piece::Queen:  s += 'q'; break;
            default: s += '?'; break;
        }

        return s;
    }

    std::uint8_t fx = 0;
    std::uint8_t fy = 0;
    std::uint8_t tx = 0;
    std::uint8_t ty = 0;
    Piece promo = Piece::None;
};

class Board
{
public:
    Board(const Board &b) :
        colors(b.colors),
        pieces(b.pieces),
        movelist_found(false),
        turn(b.turn),
        can_castle(b.can_castle),
        ep_x(b.ep_x)
    {}

    Board(std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
    {
        colors.at(static_cast<std::uint8_t>(Color::Empty)).board = ~0;

        std::vector<std::string> tokens;

        std::istringstream iss(FEN);
        std::copy(std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                std::back_inserter(tokens));

        std::uint8_t x = 0;
        std::uint8_t y = 7;

        for (const char &c : tokens.at(0))
        {
            if (c > '0' && c <= '8')
            {
                x += c-'0';
                continue;
            }

            if (c == '/')
            {
                y--;
                x = 0;
                continue;
            }

            set_tile(x, y, char_to_tile(c));
            x++;
        }

        if (tokens.at(1).at(0) == 'w')
        {
            turn = Color::White;
        }
        else
        {
            turn = Color::Black;
        }

        can_castle.at(0).at(0) = false;
        can_castle.at(0).at(1) = false;
        can_castle.at(1).at(0) = false;
        can_castle.at(1).at(1) = false;

        for (char c : tokens.at(2))
        {
            if (c == 'K')
                can_castle.at(0).at(0) = true;
            if (c == 'Q')
                can_castle.at(0).at(1) = true;
            if (c == 'k')
                can_castle.at(1).at(0) = true;
            if (c == 'q')
                can_castle.at(1).at(1) = true;
        }

        // Ensure castling ability
        {
            Tile t = get_tile(7, 0);

            if (t.color != Color::White || t.piece != Piece::Rook)
                can_castle.at(0).at(0) = false;
        }
        {
            Tile t = get_tile(0, 0);

            if (t.color != Color::White || t.piece != Piece::Rook)
                can_castle.at(0).at(1) = false;
        }
        {
            Tile t = get_tile(7, 7);

            if (t.color != Color::Black || t.piece != Piece::Rook)
                can_castle.at(1).at(0) = false;
        }
        {
            Tile t = get_tile(0, 7);

            if (t.color != Color::Black || t.piece != Piece::Rook)
                can_castle.at(1).at(1) = false;
        }
        {
            Tile t = get_tile(4, 0);

            if (t.color != Color::White || t.piece != Piece::King)
            {
                can_castle.at(0).at(0) = false;
                can_castle.at(0).at(1) = false;
            }
        }
        {
            Tile t = get_tile(4, 7);

            if (t.color != Color::Black || t.piece != Piece::King)
            {
                can_castle.at(1).at(0) = false;
                can_castle.at(1).at(1) = false;
            }
        }

        if (tokens.at(3).at(0) == '-')
        {
            ep_x = 9;
        }
        else
        {
            ep_x = tokens.at(3).at(0) - 'a';
        }
    }

    Color get_color(std::uint8_t x, std::uint8_t y) const
    {
        for (std::uint8_t c = 0; c < 3; c++)
        {
            if (colors.at(c).read(x, y))
            {
                return static_cast<Color>(c);
            }
        }

        return Color::Empty;
    }

    Piece get_piece(std::uint8_t x, std::uint8_t y) const
    {
        for (std::uint8_t p = 0; p < 6; p++)
        {
            if (pieces.at(p).read(x, y))
            {
                return static_cast<Piece>(p);
            }
        }

        return Piece::None;
    }

    Tile get_tile(std::int8_t x_, std::int8_t y_) const
    {
        if (x_ < 0 || x_ > 7 || y_ < 0 || y_ > 7)
        {
            return Tile{Color::Empty, Piece::None, true};
        }

        std::uint8_t x = static_cast<std::uint8_t>(x_);
        std::uint8_t y = static_cast<std::uint8_t>(y_);

        return Tile{get_color(x, y), get_piece(x, y)};
    }

    void set_tile(std::int8_t x_, std::int8_t y_, Tile tile)
    {
        if (x_ < 0 || x_ > 7 || y_ < 0 || y_ > 7)
        {
            std::cerr << "Set tile out of bounds (" << std::to_string(x_) << ", " << std::to_string(y_) << ")" << std::endl;
        }

        std::uint8_t x = static_cast<std::uint8_t>(x_);
        std::uint8_t y = static_cast<std::uint8_t>(y_);

        for (std::uint8_t c = 0; c < 3; c++)
        {
            colors.at(c).write(x, y, false);
        }

        for (std::uint8_t p = 0; p < 6; p++)
        {
            pieces.at(p).write(x, y, false);
        }

        colors.at(static_cast<std::uint8_t>(tile.color)).write(x, y, true);

        if (tile.piece != Piece::None)
        {
            pieces.at(static_cast<std::uint8_t>(tile.piece)).write(x, y, true);
        }
    }

    Bitboard get_bitboard(Color color, Piece piece) const
    {
        Bitboard res;
        res.board =
                colors.at(static_cast<std::uint8_t>(color)).board &
                pieces.at(static_cast<std::uint8_t>(piece)).board;
        return res;
    }

    std::vector<Move>& get_moves() const
    {
        find_movelist();

        return movelist;
    }

    Bitboard& get_threat() const
    {
        find_movelist(true);

        return threat;
    }

    Bitboard& get_enemy_threat() const
    {
        if (enemy_threat.board != 0)
            return enemy_threat;

        Board oppo(*this);

        Color enemy_turn = Color::White;
        if (turn == Color::White)
            enemy_turn = Color::Black;

        oppo.set_turn(enemy_turn);
        enemy_threat = oppo.get_threat();

        return enemy_threat;
    }

    void set_turn(Color color)
    {
        turn = color;

        movelist.clear();
        movelist_found = false;
        threat.board = 0;
        enemy_threat.board = 0;
    }

    Color get_turn() const
    {
        return turn;
    }

    void perform_move(Move move)
    {
        Tile from = get_tile(move.fx, move.fy);
        //Tile to = get_tile(move.tx, move.ty);

        if (move.promo == Piece::None)
            set_tile(move.tx, move.ty, from);
        else
            set_tile(move.tx, move.ty, Tile{from.color, move.promo});

        set_tile(move.fx, move.fy, Tile{Color::Empty, Piece::None});

        // Castling move
        if (from.piece == Piece::King && std::abs(move.tx - move.fx) >= 2)
        {
            // Kingside
            if (move.tx > move.fx)
            {
                set_tile(5, move.fy, Tile{from.color, Piece::Rook});
                set_tile(7, move.fy, Tile{Color::Empty, Piece::None});
            }

            // Queenside
            if (move.tx < move.fx)
            {
                set_tile(3, move.fy, Tile{from.color, Piece::Rook});
                set_tile(0, move.fy, Tile{Color::Empty, Piece::None});
            }
        }

        // Handle castling priviledges if king move
        if (from.piece == Piece::King)
        {
            can_castle.at(static_cast<std::uint8_t>(turn)).at(0) = false;
            can_castle.at(static_cast<std::uint8_t>(turn)).at(1) = false;
        }

        // Handle castling priviledges if rook move
        if (from.piece == Piece::Rook &&
                ((turn == Color::White && move.fy == 0) ||
                 (turn == Color::Black && move.fy == 7)))
        {
            if (move.fx == 7)
            {
                can_castle.at(static_cast<std::uint8_t>(turn)).at(0) = false;
            }

            if (move.fx == 0)
            {
                can_castle.at(static_cast<std::uint8_t>(turn)).at(1) = false;
            }
        }

        // Handle castling if rook is captured
        if (move.tx == 7 && move.ty == 0)
            can_castle.at(static_cast<std::uint8_t>(Color::White)).at(0) = false;
        if (move.tx == 0 && move.ty == 0)
            can_castle.at(static_cast<std::uint8_t>(Color::White)).at(1) = false;
        if (move.tx == 7 && move.ty == 7)
            can_castle.at(static_cast<std::uint8_t>(Color::Black)).at(0) = false;
        if (move.tx == 0 && move.ty == 7)
            can_castle.at(static_cast<std::uint8_t>(Color::Black)).at(1) = false;

        // En passant
        if (from.piece == Piece::Pawn &&
                move.fx != move.tx &&
                move.tx == ep_x)
        {
            if ((turn == Color::White && move.ty == 5) ||
                (turn == Color::Black && move.ty == 2))
            set_tile(ep_x, move.fy, Tile{Color::Empty, Piece::None});
        }

        // En passantable move
        if (from.piece == Piece::Pawn && std::abs(move.ty - move.fy) == 2)
        {
            ep_x = move.fx;
        }
        else
        {
            ep_x = 9;
        }

        if (get_turn() == Color::White)
            set_turn(Color::Black);
        else
            set_turn(Color::White);
    }

    bool is_checkmate() const
    {
        find_movelist();

        if (movelist.size() != 0)
            return false;

        get_enemy_threat();

        if ((enemy_threat.board & get_bitboard(turn, Piece::King).board) != 0)
            return true;

        return false;
    }

    bool is_stalemate() const
    {
        find_movelist();

        if ((movelist.size() != 0) || is_checkmate())
            return false;

        get_enemy_threat();

        if ((enemy_threat.board & get_bitboard(turn, Piece::King).board) == 0)
            return true;

        return false;
    }

    double basic_eval() const
    {
        find_movelist();

        if (movelist.size() == 0)
        {
            if (is_stalemate())
                return 0;
            if (turn == Color::White)
                //return -std::numeric_limits<double>::infinity();
                return -200.00;
            if (turn == Color::Black)
                return 200.00;
                //return std::numeric_limits<double>::infinity();
        }

        double eval = 0;

        for (std::uint8_t x = 0; x < 8; x++)
        {
            for (std::uint8_t y = 0; y < 8; y++)
            {
                Tile t = get_tile(x, y);

                double pv = 0;

                switch (t.piece)
                {
                    case Piece::None:   pv = 0; break;
                    case Piece::Pawn:   pv = 1; break;
                    case Piece::Knight: pv = 3; break;
                    case Piece::Bishop: pv = 3; break;
                    case Piece::Rook:   pv = 5; break;
                    case Piece::Queen:  pv = 9; break;
                    default:            pv = 0; break;
                }

                if (t.color == Color::White)
                    eval += pv;
                else if (t.color == Color::Black)
                    eval -= pv;
            }
        }

        return eval;
    }

    // Created based on "Simplified Evalution Function" on Chess Programming Wiki
    double adv_eval() const
    {
        // Piece values
        constexpr std::array<double, 6> piece_values = {1.00, 3.20, 3.30, 5.00, 9.00, 200.00};

        // Pawn Piece-Square Table
        constexpr std::array<double, 64> pawn_ps =
        {
             0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
             0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50,
             0.10, 0.10, 0.20, 0.30, 0.30, 0.20, 0.10, 0.10,
             0.05, 0.05, 0.10, 0.25, 0.25, 0.10, 0.05, 0.05,
             0.00, 0.00, 0.00, 0.20, 0.20, 0.00, 0.00, 0.00,
             0.05,-0.05,-0.10, 0.00, 0.00,-0.10,-0.05, 0.05,
             0.05, 0.10, 0.10,-0.20,-0.20, 0.10, 0.10, 0.05,
             0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
        };

        // Knight Piece-Square Table
        constexpr std::array<double, 64> knight_ps =
        {
            -0.50,-0.40,-0.30,-0.30,-0.30,-0.30,-0.40,-0.50,
            -0.40,-0.20, 0.00, 0.00, 0.00, 0.00,-0.20,-0.40,
            -0.30, 0.00, 0.10, 0.15, 0.15, 0.10, 0.00,-0.30,
            -0.30, 0.05, 0.15, 0.20, 0.20, 0.15, 0.05,-0.30,
            -0.30, 0.00, 0.15, 0.20, 0.20, 0.15, 0.00,-0.30,
            -0.30, 0.05, 0.10, 0.15, 0.15, 0.10, 0.05,-0.30,
            -0.40,-0.20, 0.00, 0.05, 0.05, 0.00,-0.20,-0.40,
            -0.50,-0.40,-0.30,-0.30,-0.30,-0.30,-0.40,-0.50
        };

        // Bishop Piece-Square Table
        constexpr std::array<double, 64> bishop_ps =
        {
            -0.20,-0.10,-0.10,-0.10,-0.10,-0.10,-0.10,-0.20,
            -0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.10,
            -0.10, 0.00, 0.05, 0.10, 0.10, 0.05, 0.00,-0.10,
            -0.10, 0.05, 0.05, 0.10, 0.10, 0.05, 0.05,-0.10,
            -0.10, 0.00, 0.10, 0.10, 0.10, 0.10, 0.00,-0.10,
            -0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10,-0.10,
            -0.10, 0.05, 0.00, 0.00, 0.00, 0.00, 0.05,-0.10,
            -0.20,-0.10,-0.10,-0.10,-0.10,-0.10,-0.10,-0.20
        };

        // Rook Piece-Square Table
        constexpr std::array<double, 64> rook_ps =
        {
             0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
             0.05, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.05,
            -0.05, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.05,
            -0.05, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.05,
            -0.05, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.05,
            -0.05, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.05,
            -0.05, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.05,
             0.00, 0.00, 0.00, 0.05, 0.05, 0.00, 0.00, 0.00
        };

        // Queen Piece-Square Table
        constexpr std::array<double, 64> queen_ps =
        {
            -0.20,-0.10,-0.10,-0.05,-0.05,-0.10,-0.10,-0.20,
            -0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,-0.10,
            -0.10, 0.00, 0.05, 0.05, 0.05, 0.05, 0.00,-0.10,
            -0.05, 0.00, 0.05, 0.05, 0.05, 0.05, 0.00,-0.05,
             0.00, 0.00, 0.05, 0.05, 0.05, 0.05, 0.00,-0.05,
            -0.10, 0.05, 0.05, 0.05, 0.05, 0.05, 0.00,-0.10,
            -0.10, 0.00, 0.05, 0.00, 0.00, 0.00, 0.00,-0.10,
            -0.20,-0.10,-0.10,-0.05,-0.05,-0.10,-0.10,-0.20
        };

        // King middle-game Piece-Square Table
        constexpr std::array<double, 64> king_middle_ps =
        {
            -0.30,-0.40,-0.40,-0.50,-0.50,-0.40,-0.40,-0.30,
            -0.30,-0.40,-0.40,-0.50,-0.50,-0.40,-0.40,-0.30,
            -0.30,-0.40,-0.40,-0.50,-0.50,-0.40,-0.40,-0.30,
            -0.30,-0.40,-0.40,-0.50,-0.50,-0.40,-0.40,-0.30,
            -0.20,-0.30,-0.30,-0.40,-0.40,-0.30,-0.30,-0.20,
            -0.10,-0.20,-0.20,-0.20,-0.20,-0.20,-0.20,-0.10,
             0.20, 0.20, 0.00, 0.00, 0.00, 0.00, 0.20, 0.20,
             0.20, 0.30, 0.10, 0.00, 0.00, 0.10, 0.30, 0.20
        };

        // King end-game Piece-Square Table
        constexpr std::array<double, 64> king_end_ps =
        {
            -0.50,-0.40,-0.30,-0.20,-0.20,-0.30,-0.40,-0.50,
            -0.30,-0.20,-0.10, 0.00, 0.00,-0.10,-0.20,-0.30,
            -0.30,-0.10, 0.20, 0.30, 0.30, 0.20,-0.10,-0.30,
            -0.30,-0.10, 0.30, 0.40, 0.40, 0.30,-0.10,-0.30,
            -0.30,-0.10, 0.30, 0.40, 0.40, 0.30,-0.10,-0.30,
            -0.30,-0.10, 0.20, 0.30, 0.30, 0.20,-0.10,-0.30,
            -0.30,-0.30, 0.00, 0.00, 0.00, 0.00,-0.30,-0.30,
            -0.50,-0.30,-0.30,-0.30,-0.30,-0.30,-0.30,-0.50
        };

        find_movelist();

        if (is_stalemate())
            return 0;

        if (is_checkmate())
        {
            if (turn == Color::White)
                return -200.00;

            if (turn == Color::Black)
                return 200.00;
        }

        double eval = 0;

        // 0 is middle game, 1 is end game, can interpolate between
        double endgameness = 0;

        {
            Bitboard white_pieces;
            Bitboard black_pieces;
            white_pieces.board = colors.at(static_cast<std::uint8_t>(Color::White)).board - get_bitboard(Color::White, Piece::Pawn).board;
            black_pieces.board = colors.at(static_cast<std::uint8_t>(Color::Black)).board - get_bitboard(Color::Black, Piece::Pawn).board;

            if (
                    ((get_bitboard(Color::White, Piece::Queen).board == 0) || (white_pieces.count() <= 1)) ||
                    ((get_bitboard(Color::Black, Piece::Queen).board == 0) || (black_pieces.count() <= 1))
               )
            {
                endgameness = 1;
            }
        }

        for (std::uint8_t x = 0; x < 8; x++)
        {
            for (std::uint8_t y = 0; y < 8; y++)
            {
                Tile t = get_tile(x, y);

                if (t.piece == Piece::None)
                    continue;

                std::uint8_t index = (7-y)*8+x; // For Piece-Square Tables for white
                if (t.color == Color::Black)
                    index = y*8+x; // For black (reflected, not rotated)

                double pv = piece_values.at(static_cast<std::uint8_t>(t.piece));

                switch (t.piece)
                {
                    case Piece::Pawn:   pv += pawn_ps.at(index); break;
                    case Piece::Knight: pv += knight_ps.at(index); break;
                    case Piece::Bishop: pv += bishop_ps.at(index); break;
                    case Piece::Rook:   pv += rook_ps.at(index); break;
                    case Piece::Queen:  pv += queen_ps.at(index); break;
                    case Piece::King:   pv += endgameness*king_end_ps.at(index) + (1-endgameness)*king_middle_ps.at(index); break;
                    default: break;
                }

                if (t.color == Color::White)
                    eval += pv;
                else
                    eval -= pv;
            }
        }

        return eval;
    }

    void print(std::ostream &os = std::cout) const
    {
        std::string s(10*11, ' ');

        for (std::uint8_t i = 0; i < 10; i++)
            s.at(i*11+10) = '\n';

        for (std::uint8_t x = 0; x < 8; x++)
        {
            for (std::uint8_t y = 0; y < 8; y++)
            {
                std::uint8_t pos = ((8-y)*11)+x+1;

                s.at(pos) = tile_to_char(get_tile(x, y));
            }
        }

        if (turn == Color::White)
            os << "White to move\n";
        else
            os << "Black to move\n";

        os << s;

        /*
        std::cout << "Colors (white/black/empty)\n";

        for (std::uint8_t c = 0; c < 3; c++)
        {
            colors.at(c).print();
            std::cout << '\n';
        }

        std::cout << "Pieces (p,k,b,r,q,k)\n";

        for (std::uint8_t p = 0; p < 6; p++)
        {
            pieces.at(p).print();
            std::cout << '\n';
        }
        */

        os << "Can castle: ";
        if (can_castle.at(0).at(0))
            os << 'K';
        else
            os << '-';

        if (can_castle.at(0).at(1))
            os << 'Q';
        else
            os << '-';

        if (can_castle.at(1).at(0))
            os << 'k';
        else
            os << '-';

        if (can_castle.at(1).at(1))
            os << 'q';
        else
            os << '-';

        os << std::endl;
    }

private:
    void find_movelist(bool find_threat = false) const
    {
        if (movelist_found)
            return;

        if (find_threat && threat.board != 0)
            return;

        Color player = get_turn();
        Color enemy = Color::White;
        if (player == Color::White)
            enemy = Color::Black;

        if (find_threat)
        {
            threat.board = 0;
        }
        else
        {
            enemy_threat = get_enemy_threat();
        }

        std::vector<Move> moves; // This will include pseudolegal moves

        // For every tile ...
        for (std::uint8_t x = 0; x < 8; x++)
        {
            for (std::uint8_t y = 0; y < 8; y++)
            {
                const Tile tile = get_tile(x, y);

                // Check if the piece is correct color
                if (player != tile.color)
                {
                    continue;
                }

                // Quick method to expand pawn moves into all promotions
                const auto pawn_moves = [&](std::uint8_t fx, std::uint8_t fy, std::uint8_t tx, std::uint8_t ty)
                {
                    if (!find_threat)
                    {
                        if (ty == 7 || ty == 0)
                        {
                            moves.emplace_back(fx, fy, tx, ty, Piece::Knight);
                            moves.emplace_back(fx, fy, tx, ty, Piece::Bishop);
                            moves.emplace_back(fx, fy, tx, ty, Piece::Rook);
                            moves.emplace_back(fx, fy, tx, ty, Piece::Queen);
                        }
                        else
                        {
                            moves.emplace_back(fx, fy, tx, ty);
                        }
                    }
                };

                // Quick method for iterating over knight moves
                const auto km = [](std::uint8_t i, std::uint8_t n)
                {
                    if (n == 0)
                    {
                        switch (i)
                        {
                            case 0: return -1; break;
                            case 1: return  1; break;
                            case 2: return  2; break;
                            case 3: return  2; break;
                            case 4: return  1; break;
                            case 5: return -1; break;
                            case 6: return -2; break;
                            case 7: return -2; break;
                            default: return 0; break;
                        }
                    }
                    else
                    {
                        switch (i)
                        {
                            case 0: return  2; break;
                            case 1: return  2; break;
                            case 2: return  1; break;
                            case 3: return -1; break;
                            case 4: return -2; break;
                            case 5: return -2; break;
                            case 6: return -1; break;
                            case 7: return  1; break;
                            default: return 0; break;
                        }
                    }
                };

                // Quick method for iterating over bishop moves
                const auto bm = [](std::uint8_t d, std::uint8_t n)
                {
                    if (n == 0)
                    {
                        switch (d)
                        {
                            case 0: return  1; break;
                            case 1: return  1; break;
                            case 2: return -1; break;
                            case 3: return -1; break;
                            default: std::cout << "??" << std::endl; return 1; break;
                        }
                    }
                    else if (n == 1)
                    {
                        switch (d)
                        {
                            case 0: return  1; break;
                            case 1: return -1; break;
                            case 2: return  1; break;
                            case 3: return -1; break;
                            default: std::cout << "??" << std::endl; return 1; break;
                        }
                    }
                    else
                    {
                        std::cout << "!?!" << std::endl;
                        return 1;
                    }
                };

                // Quick method for iterating over rook moves
                const auto rm = [](std::uint8_t d, std::uint8_t n)
                {
                    switch (d)
                    {
                        case 0: return n ?  1 :  0; break;
                        case 1: return n ?  0 :  1; break;
                        case 2: return n ? -1 :  0; break;
                        case 3: return n ?  0 : -1; break;
                        default: return 1; break;
                    }
                };

                switch (tile.piece)
                {
                    case Piece::Pawn:
                        {
                            if (player == Color::White)
                            {
                                // Standard pawn move
                                if (get_tile(x, y+1).color == Color::Empty)
                                {
                                    pawn_moves(x, y, x, y+1);

                                    // If at starting pos
                                    if (y == 1 && get_tile(x, y+2).color == Color::Empty)
                                    {
                                        pawn_moves(x, y, x, y+2);
                                    }
                                }

                                // Attacking moves
                                {
                                    const Tile t1 = get_tile(x-1, y+1);
                                    if (!t1.oob)
                                    {
                                        threat.write(x-1, y+1, true);

                                        if ((t1.color == enemy) ||
                                                (y == 4 && ep_x == x-1)) // En passant
                                        {
                                            pawn_moves(x, y, x-1, y+1);
                                        }
                                    }

                                    const Tile t2 = get_tile(x+1, y+1);
                                    if (!t2.oob)
                                    {
                                        threat.write(x+1, y+1, true);

                                        if ((t2.color == enemy) ||
                                                (y == 4 && ep_x == x+1)) // En passant
                                        {
                                            pawn_moves(x, y, x+1, y+1);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // Standard pawn move
                                if (get_tile(x, y-1).color == Color::Empty)
                                {
                                    pawn_moves(x, y, x, y-1);

                                    // If at starting pos
                                    if (y == 6 && get_tile(x, y-2).color == Color::Empty)
                                    {
                                        pawn_moves(x, y, x, y-2);
                                    }
                                }

                                // Attacking moves
                                {
                                    const Tile t1 = get_tile(x-1, y-1);
                                    if (!t1.oob)
                                    {
                                        threat.write(x-1, y-1, true);

                                        if ((t1.color == enemy) ||
                                                (y == 3 && ep_x == x-1)) // En passant
                                        {
                                            pawn_moves(x, y, x-1, y-1);
                                        }
                                    }

                                    const Tile t2 = get_tile(x+1, y-1);
                                    if (!t2.oob)
                                    {
                                        threat.write(x+1, y-1, true);

                                        if ((t2.color == enemy) ||
                                                (y == 3 && ep_x == x+1)) // En passant
                                        {
                                            pawn_moves(x, y, x+1, y-1);
                                        }
                                    }
                                }
                            }
                        }
                        break;

                    case Piece::Knight:
                        {
                            for (std::uint8_t i = 0; i < 8; i++)
                            {
                                std::int8_t x_ = x+km(i,0);
                                std::int8_t y_ = y+km(i,1);

                                Tile t = get_tile(x_, y_);
                                if (!t.oob)
                                {
                                    threat.write(x_, y_, true);

                                    if (t.color != player)
                                    {
                                        moves.emplace_back(x, y, x_, y_);
                                    }
                                }
                            }
                        }
                        break;

                    case Piece::Bishop:
                        {
                            for (std::uint8_t d = 0; d < 4; d++)
                            {
                                for (std::uint8_t i = 1; ; i++)
                                {
                                    std::int8_t x_ = x+i*bm(d,0);
                                    std::int8_t y_ = y+i*bm(d,1);

                                    Tile t = get_tile(x_, y_);

                                    //if (player == Color::White)
                                        //std::cout << "\n. " << std::to_string(x_) << " " << std::to_string(y_);

                                    if (t.oob)
                                        break;

                                    threat.write(x_, y_, true);

                                    if (t.color == player)
                                        break;

                                    //if (player == Color::White)
                                        //std::cout << '!';

                                    moves.emplace_back(x, y, x_, y_);

                                    if (t.color == enemy)
                                        break;

                                    //if (player == Color::White)
                                        //std::cout << '?';
                                }
                            }
                        }
                        break;

                    case Piece::Rook:
                        {
                            for (std::uint8_t d = 0; d < 4; d++)
                            {
                                for (std::uint8_t i = 1; ; i++)
                                {
                                    std::int8_t x_ = x+i*rm(d,0);
                                    std::int8_t y_ = y+i*rm(d,1);

                                    Tile t = get_tile(x_, y_);

                                    if (t.oob)
                                        break;

                                    threat.write(x_, y_, true);

                                    if (t.color == player)
                                        break;

                                    moves.emplace_back(x, y, x_, y_);

                                    if (t.color == enemy)
                                        break;
                                }
                            }
                        }
                        break;

                    case Piece::Queen:
                        {
                            // Bishop moves
                            for (std::uint8_t d = 0; d < 4; d++)
                            {
                                for (std::uint8_t i = 1; ; i++)
                                {
                                    std::int8_t x_ = x+i*bm(d,0);
                                    std::int8_t y_ = y+i*bm(d,1);

                                    Tile t = get_tile(x_, y_);

                                    if (t.oob)
                                        break;

                                    threat.write(x_, y_, true);

                                    if (t.color == player)
                                        break;

                                    moves.emplace_back(x, y, x_, y_);

                                    if (t.color == enemy)
                                        break;
                                }
                            }

                            // Rook moves
                            for (std::uint8_t d = 0; d < 4; d++)
                            {
                                for (std::uint8_t i = 1; ; i++)
                                {
                                    std::int8_t x_ = x+i*rm(d,0);
                                    std::int8_t y_ = y+i*rm(d,1);

                                    Tile t = get_tile(x_, y_);

                                    if (t.oob)
                                        break;

                                    threat.write(x_, y_, true);

                                    if (t.color == player)
                                        break;

                                    moves.emplace_back(x, y, x_, y_);

                                    if (t.color == enemy)
                                        break;
                                }
                            }
                        }
                        break;

                    case Piece::King:
                        {
                            for (std::int8_t x_ = x-1; x_ <= x+1; x_++)
                            {
                                for (std::int8_t y_ = y-1; y_ <= y+1; y_++)
                                {
                                    if (x_ == x && y_ == y)
                                        continue;

                                    Tile t = get_tile(x_, y_);

                                    if (t.oob)
                                        continue;

                                    threat.write(x_, y_, true);

                                    if (t.color == player)
                                        continue;

                                    moves.emplace_back(x, y, x_, y_);
                                }
                            }

                            //King threatens his own square (important I suspect for checking if enemy threat has been found)
                            threat.write(x, y, true);

                            // Castling moves don't threat
                            if (!find_threat)
                            {
                                // Kingside castling
                                if (can_castle.at(static_cast<std::uint8_t>(player)).at(0))
                                {
                                    bool clear = true;

                                    for (std::uint8_t x_ = 6; x_ >= 5; x_--)
                                    {
                                        if (get_tile(x_, y).color != Color::Empty)
                                        {
                                            clear = false;
                                            break;
                                        }
                                    }

                                    if (clear)
                                    {
                                        Bitboard king_squares;
                                        king_squares.write(4, y, true);
                                        king_squares.write(5, y, true);
                                        king_squares.write(6, y, true);

                                        if ((enemy_threat.board & king_squares.board) == 0)
                                        {
                                            moves.emplace_back(x, y, 6, y);
                                        }
                                    }
                                }

                                // Queenside castling
                                if (can_castle.at(static_cast<std::uint8_t>(player)).at(1))
                                {
                                    bool clear = true;

                                    for (std::uint8_t x_ = 1; x_ <= 3; x_++)
                                    {
                                        if (get_tile(x_, y).color != Color::Empty)
                                        {
                                            clear = false;
                                            break;
                                        }
                                    }

                                    if (clear)
                                    {
                                        Bitboard king_squares;
                                        king_squares.write(2, y, true);
                                        king_squares.write(3, y, true);
                                        king_squares.write(4, y, true);

                                        if ((enemy_threat.board & king_squares.board) == 0)
                                        {
                                            moves.emplace_back(x, y, 2, y);
                                        }
                                    }
                                }
                            }
                        }
                        break;

                    default:
                        {
                            std::cerr << "Unhandled piece type for finding move!" << std::endl;
                        }
                        break;
                }
            }
        }

        for (const Move &move : moves)
        {
            threat.write(move.tx, move.ty, true);
        }

        if (!find_threat)
        {
            Bitboard king_tile = get_bitboard(player, Piece::King);

            bool under_threat = static_cast<bool>(enemy_threat.board & king_tile.board);

            for (const Move &move : moves)
            {
                Piece from = get_piece(move.fx, move.fy);
                Bitboard tile_bb;
                tile_bb.write(move.fx, move.fy, true);

                // Remove moves where kings walks into check
                if (from == Piece::King)
                {
                    Bitboard king_move;
                    king_move.write(move.tx, move.ty, true);

                    if ((enemy_threat.board & king_move.board) != 0)
                    {
                        continue;
                    }
                }

                // Check moves that cannot be assumed safe (en passant must always be checked)
                if (under_threat ||
                        (enemy_threat.board & tile_bb.board) ||
                        (from == Piece::Pawn && (move.fx != move.tx)))
                {
                    Board next(*this);

                    next.perform_move(move);

                    Bitboard next_threat = next.get_threat();

                    if ((next.get_bitboard(player, Piece::King).board & next_threat.board) != 0)
                    {
                        continue;
                    }
                }

                movelist.push_back(move);
            }

            movelist_found = turn == player && !find_threat;
        }
    }

    std::array<Bitboard, 3> colors;
    std::array<Bitboard, 6> pieces;

    mutable Bitboard threat;
    mutable Bitboard enemy_threat;
    mutable bool movelist_found = false;
    mutable std::vector<Move> movelist;

    Color turn = Color::White;
    std::array<std::array<bool, 2>, 2> can_castle; // KQkq
    std::uint8_t ep_x = 9; // x value for en passant, 9 if no en passant
};

class BoardTree
{
public:
    BoardTree(const Board &parent_board)
        : board(parent_board)
    {
    }

    BoardTree(const Board &parent_board, const Move &parent_move)
        : board(parent_board), move(parent_move)
    {
        board.perform_move(move);
    }

    void expand(std::uint8_t n = 1)
    {
        std::vector<Move> &moves = board.get_moves();

        if (!expanded)
        {
            nodes.reserve(moves.size());

            for (const Move &m : moves)
            {
                nodes.emplace_back(board, m);
            }

            expanded = true;
        }

        if (n != 1)
        {
            for (BoardTree &t : nodes)
            {
                t.expand(n-1);
            }
        }
    }

    std::uint64_t depth(std::uint8_t d)
    {
        if (d == 0)
            return 1;

        expand();

        std::uint64_t n = 0;
        for (BoardTree &b : nodes)
            n += b.depth(d-1);

        nodes.clear();
        expanded = false;

        return n;
    }

    double evaluation;
    Move bestmove = Move(0, 0, 0, 0);

    std::vector<BoardTree> nodes;
    bool expanded = false;

    Board board;
    Move move = Move(0, 0, 0, 0);
};

#endif
