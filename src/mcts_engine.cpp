#include "UCIEngine.hpp"

#include <functional>
#include <cmath>

class MctsEngine : public UCIEngine
{
public:
    MctsEngine()
    {
        engine_author = "Mathias Lyngbye";
        engine_name = "MCTS Engine";
        start();
    }

    MoveList movelist;

    void think() override
    {
        // Set turn bias
        turn_bias = 1;
        if(board.get_turn() == Color::Black)
            turn_bias = -1;

        // Initialization
        MoveList startmoves;
        board.get_moves(startmoves);
        bestmove = startmoves.at(1);

        // Root node
        BoardTree root_node(board);

        // MCTS
        uint64_t time_max = 2000;
        while(time_spent < time_max)
        {
            // === Selection ===
            BoardTree* promising_node = select_node(&root_node);

            // === Expansion ===
            promising_node->expand(movelist);
            BoardTree* node_to_explore = promising_node;
            
            // If expansion yielded children, pick a random child
            if(promising_node->nodes.size() > 0)
            {
                std::uniform_int_distribution<std::uint8_t> dist(0, promising_node->nodes.size()-1);
                node_to_explore = &promising_node->nodes.at(dist(eng));
            }

            // === Simulation ===
            int playout_result = simulate_playout_random(node_to_explore); // UNBIASED

            // === Backpropagation ===
            // I HAVE AN INHERENT PROBLEM OF NOT CONSIDERING TURNS

        }

        // End of function
        thinking = false;
    }

    // Input:  Tree node
    // Output: Normalized unbiased end state evaluation (1,0,-1)
    int simulate_playout_random(BoardTree *root_node)
    {
        MoveList tmp_moves;
        
        Board tmp_board = root_node->board;
        tmp_board.get_moves(tmp_moves);
        
        // Find end state (win/lose/draw/max iterations)
        int max_moves = 150;
        int moves = 0;
        while( !tmp_board.is_checkmate(tmp_moves) 
            && !tmp_board.is_stalemate(tmp_moves) 
            && moves < max_moves )
        {
            // Generate random move
            std::uniform_int_distribution<std::uint8_t> dist(0, tmp_moves.size()-1);
            Move random_move = tmp_moves.at(dist(eng));
            
            // Perform move and get moves of next state
            tmp_board.perform_move(random_move);
            tmp_board.get_moves(tmp_moves);
            
            // Ensure max iterations
            moves++;
        }

        // Evaluate end state (this needs turn bias?)
        int eval = tmp_board.basic_eval(tmp_moves);             // Rough double to int conversion
        int eval_norm = (eval/std::abs(eval));                  // Normalized evaluation (1,0,-1)
        
        return eval_norm;
    }

    void backpropagation(BoardTree* node, int playout_result)
    {
        BoardTree* eval_node = node;
        while(eval_node != NULL)
        {
            eval_node->visitcount++;
        }
    }

    BoardTree *select_node(BoardTree *root_node)
    {
        BoardTree *node = root_node;
        while(node->nodes.size() != 0)  // While not a leaf
        {
            node = find_best_child(node);
        }
        
        return node;
    }

    BoardTree *find_best_child(BoardTree *node)
    {
        int parent_visitcount = node->visitcount;
        const auto comparator = [&](const BoardTree &a, const BoardTree &b)
        {
            double node_a_uct = uct(parent_visitcount, a.winscore, a.visitcount);
            double node_b_uct = uct(parent_visitcount, b.winscore, b.visitcount);
            return (node_a_uct < node_b_uct);
        };

        return &(*std::max_element(node->nodes.begin(), node->nodes.end(), comparator));
    }

    double uct(int parent_visit, int node_visit, double node_score)
    {
        double c = std::sqrt(2.0); // Theoretical value
        return ((node_score / node_visit) + c * std::sqrt(std::log(parent_visit) / node_visit));
    }

    // Member variables
    int turn_bias = 0;  // Needed for evals
    int win_score = 10;
};

int main()
{
    MctsEngine engine;

    return 0;
}