#include "UCIEngine.hpp"

class MinimaxEngine : public UCIEngine
{
public:
    MinimaxEngine()
    {
        start();
    }

    void think() override
    {
        // Default white
        int turn = 1;
        if(board.get_turn() == Color::Black)
            turn = -1;

        std::vector<Move> moves = board.get_moves();

        if (moves.size() == 0)
        {
            std::cerr << "No legal moves found!" << std::endl;
        }
        else
        {
            for (const Move &move : moves)
                log << move.longform() << std::endl;
        }

        // Find random move among moves
        std::shuffle(moves.begin(), moves.end(), eng);
        Board test_board(board);
        test_board.perform_move(moves.at(0));
        double bestvalue = test_board.adv_eval() * turn;
        bestmove = moves.at(0);     // Default first move in case rest fails
        for (const Move &move : moves)
        {
            test_board = board;
            test_board.perform_move(move);

            double eval = test_board.adv_eval() * turn;
            if(eval >= bestvalue)
            {
                bestvalue = eval;
                bestmove = move;
            }
        }
        std::cout << "Best value: " << bestvalue << std::endl;
        
        // End of function
        thinking = false;
    }
};

int main()
{
    MinimaxEngine engine;

    return 0;
}
