#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>

const int GRID_COLS = 11;
const int GRID_ROWS = 5;
int TOTAL_COLS = GRID_COLS * GRID_ROWS;
unsigned long long failures = 0;
unsigned long long solutions = 0;

struct node 
{
    int indexC = 0;
    int indexR = 0;
    int value = 0;
    bool deleted = true;
    bool filled = false;
    node* left = nullptr;
    node* right = nullptr;
    node* up = nullptr;
    node* down = nullptr;
};

node grid[GRID_ROWS][GRID_COLS]; //piece needs to see the grid

struct piece 
{
    int stateID = -1;
    int space = 0;
    int width = 0;
    int height = 0;
    char output = '.';
    int rot = 0;
    bool flip = false;
    std::string state;
    std::vector<int> indexes;
    piece() {}
    piece(int startSpace, char _output, int _width, int _height, std::string _state)
        : space(startSpace), output(_output), width(_width), height(_height), state(_state)
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if(state[i*width + j] == '1')
                    indexes.push_back(i*GRID_COLS + j);
            }
        }
    }
    void rotateRight()
    {
        int oldWidth = width;
        int oldHeight = height;
        width = oldHeight;
        height = oldWidth;
        std::string oldState = state;
        int counter = 0;
        for (int c = width - 1; c >= 0; c--)
        {
            for (int r = 0; r < height; r++)
                state[r*width + c] = oldState[counter++];
        }

        indexes.clear();
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (state[i*width + j] == '1')
                    indexes.push_back(i*GRID_COLS + j);
            }
        }
        rot = rot + 1;
    }
    void flipHorizontal()
    {
        std::string oldState = state;
        int counter = 0;
        for (int r = 0; r < height; r++)
        {
            for (int c = width - 1; c >= 0; c--)
                state[r*width + c] = oldState[counter++];
        }

        indexes.clear();
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (state[i*width + j] == '1')
                    indexes.push_back(i*GRID_COLS + j);
            }
        }
        flip = !flip;
    }
    bool moveRight()
    {
        for (int i = 0; i < indexes.size(); i++)
        {
            int index = indexes[i];
            node * cell = &grid[index / GRID_COLS][index%GRID_COLS];
            if (cell->right == nullptr)
                return false;
        }

        for (int i = 0; i < indexes.size(); i++)
        {
            indexes[i]++;
        }
        return true;
    }
    bool moveNext()
    {
        if (!moveRight())
        {
            std::vector<int> oldIndexes = indexes;
            bool good = false;
            while (!good)
            {
                for (int i = 0; i < indexes.size(); i++)
                {
                    if (indexes[i] + 1 >= GRID_COLS * GRID_ROWS)
                    {
                        indexes = oldIndexes;
                        return false;
                    }
                }

                for (int i = 0; i < indexes.size(); i++)
                {
                    indexes[i]++;
                }

                good = true;
                for (int i = 0; i < indexes.size(); i++)
                {
                    int index = indexes[i];
                    node * cell = &grid[index / GRID_COLS][index%GRID_COLS];
                    if (cell->right == nullptr)
                        good = false;
                }
            }
            return true;
        }
        else
            return true;
    }
};

std::vector<piece> pieces;
std::vector<piece> pieceStates;
int Xgrid[GRID_COLS*GRID_ROWS] = { 0 };

void rebuildXgrid(std::set<int> deletedRows)
{
    for (int i = 0; i < GRID_COLS * GRID_ROWS; i++)
        Xgrid[i] = 0;
    for (auto ps : pieceStates)
    {
        for (auto psi : ps.indexes)
            if (deletedRows.find(ps.stateID) == deletedRows.end())
                Xgrid[psi]++;
    }
}

bool scanXgridFor0(std::set<int> deletedCols)
{
    for (int i = 0; i < GRID_COLS * GRID_ROWS; i++)
    {
        if (deletedCols.find(i) == deletedCols.end() && Xgrid[i] == 0)
            return true;
    }
    return false;
}

void xstep5(piece state, std::vector<piece> &workingSet, std::set<int> &deletedRows, std::set<int> &deletedCols)
{
    //Step 5
    //Delete all the rows with the given indexes from this piece and update column totals
    std::vector<int> selIndexes = state.indexes;
    for (int i = 0; i < selIndexes.size(); i++)
    {
        deletedCols.emplace(selIndexes[i]);
        for (std::vector<piece>::iterator iter = workingSet.begin(); iter != workingSet.end(); )
        {
            bool matching = false;
            for (auto pi : iter->indexes)
            {
                if (selIndexes[i] == pi)
                {
                    matching = true;
                    break;
                }
            }
            if (iter->output == state.output)
            {
                matching = true;
            }
            if (matching)
            {
                deletedRows.emplace(iter->stateID);
                iter = workingSet.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    rebuildXgrid(deletedRows);
}

std::vector<piece> getLeastRows(std::vector<piece> workingSet, std::set<int> deletedRows, std::set<int> deletedCols)
{
    rebuildXgrid(deletedRows);
    // Level 0 -> Get lowest column
    int lowestCol = TOTAL_COLS;
    int lowest = workingSet.size() + 1;
    for (int i = 0; i < TOTAL_COLS; i++)
    {
        if (deletedCols.find(i) == deletedCols.end() && Xgrid[i] < lowest)
        {
            lowest = Xgrid[i];
            lowestCol = i;
        }
    }

    std::vector<piece> lowestStates;
    for (auto p : workingSet)
    {
        for (auto pi : p.indexes)
        {
            if (pi == lowestCol)
                lowestStates.push_back(p);
        }
    }
    return lowestStates;
}

void printSolution(std::vector<piece> solution)
{
    std::cout << std::endl;
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            char output = '.';
            for (auto p : solution)
            {
                for (auto pi : p.indexes)
                {
                    if (pi == i*GRID_COLS + j)
                    {
                        output = p.output;
                        break;
                    }
                }
            }
            std::cout << output;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void findSolution(std::vector<piece> solution, std::vector<piece> workingPieces, std::set<int> deletedCols, std::set<int> deletedRows)
{
    //Status update
    std::cout << "\rF: " << failures << " S: " << solutions;

    //Step 1
    if (deletedCols.size() == TOTAL_COLS)
    {
        printSolution(solution);
        std::set<int> solutionPieceIDs;
        for (auto sp : solution)
            solutionPieceIDs.emplace(sp.stateID);
        if (solutionPieceIDs.size() == pieces.size())
        {
            solutions++;
        }
        return;
    }

    //Step 2
    std::vector<piece> least = getLeastRows(workingPieces, deletedRows, deletedCols);

    //Step 3
    for (int i = 0; i < least.size(); i++)
    {
        //Step 4
        std::vector<piece> newSolution = solution;
        newSolution.push_back(least[i]);

        //Step 5
        std::vector<piece> workingSet = workingPieces;
        std::set<int> newDeletedCols = deletedCols;
        std::set<int> newDeletedRows = deletedRows;
        xstep5(least[i], workingSet, newDeletedRows, newDeletedCols);
        if (scanXgridFor0(newDeletedCols))
        {
            failures++;
            return;
        }

        //Step 6
        findSolution(newSolution, workingSet, newDeletedCols, newDeletedRows);
    }
}

int main()
{
    //Future TODO: Read in pieces
    pieces.push_back(piece(4, 'A', 2, 3, "010111"));
    pieces.push_back(piece(5, 'B', 2, 3, "011111"));
    pieces.push_back(piece(5, 'C', 2, 4, "01010111"));
    pieces.push_back(piece(5, 'D', 2, 4, "01011101"));
    pieces.push_back(piece(5, 'E', 2, 4, "01011110"));
    pieces.push_back(piece(3, 'F', 2, 2, "1011"));
    pieces.push_back(piece(5, 'G', 3, 3, "001001111"));
    pieces.push_back(piece(5, 'H', 3, 3, "001011110"));
    pieces.push_back(piece(4, 'I', 3, 2, "101111"));
    pieces.push_back(piece(4, 'J', 1, 4, "1111"));
    pieces.push_back(piece(5, 'K', 2, 2, "1111"));
    pieces.push_back(piece(5, 'L', 3, 3, "010111010"));

    //Generate game grid
    for (int i = 0; i < GRID_ROWS; i++)
        for (int j = 0; j < GRID_COLS; j++)
        {
            grid[i][j].indexC = i*GRID_COLS + j;
            if(i-1 >= 0)
                grid[i][j].up = &grid[i-1][j];
            if(i+1 < GRID_ROWS)
                grid[i][j].down = &grid[i+1][j];
            if(j-1 >= 0)
                grid[i][j].left = &grid[i][j-1];
            if(j+1 < GRID_COLS)
                grid[i][j].right = &grid[i][j+1];
        }
    
    //Find all sets of indexes (all possible piece states)
    int counter = 0;
    pieceStates = pieces; //copy original piece states
    for (int i = 0; i < pieces.size(); i++)
        pieceStates[i].stateID = counter++;
    bool done = false;
    while (!done)
    {
        //Iterate through each piece's possible configurations
        for (int i = 0; i < pieces.size(); i++)
        {
            //if can't move, flip
            if (pieces[i].moveNext())
            {
                pieces[i].stateID = counter++;
                pieceStates.push_back(pieces[i]);
                break;
            }
            //if flipped, rotate
            if (!pieces[i].flip)
            {
                pieces[i].flipHorizontal();
                pieces[i].stateID = counter++;
                pieceStates.push_back(pieces[i]);
                break;
            }
            //if reached end of rotations, continue to next piece
            if (pieces[i].rot < 4)
            {
                if (pieces[i].rot != 3)
                {
                    pieces[i].rotateRight();
                    pieces[i].flip = false;
                    pieces[i].stateID = counter++;
                    pieceStates.push_back(pieces[i]);
                    break;
                }
            }
            if(i == pieces.size()-1)
                done = true;
        }
    }

    //Algorithm X - generate universe matrix with no deleted columns
    rebuildXgrid(std::set<int>());

    //Master search
    findSolution(std::vector<piece>(), pieceStates, std::set<int>(), std::set<int>());

    std::cout << "\n\nDONE.";
    return 0;
}

