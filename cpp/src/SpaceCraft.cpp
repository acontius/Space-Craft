#include "../SpaceCraft.hpp"
#include <iostream>
#include <fstream>
#include <vector>
    
using namespace std;

const vector<vector<pair<int, int>>> SpaceCraft::allDirections = {
    {{-1, 0}, {0 ,  1}, {1, 0}, {0, -1}},
    {{0,  1}, {-1 , 0}, {0, -1}, {1 , 0}},
    {{-1, 0}, {0 , -1}, {1, 0}, {0,  1}},
    {{0 , 1}, {1 , 0}, {0, -1}, {-1, 0}},
    {{1 , 0}, {0 ,  1}, {-1, 0}, {0, -1}},
    {{1 , 0}, {0 , -1}, {-1, 0}, {0 , 1}},
    {{0, -1}, {-1 , 0}, {0,  1}, {1 , 0}},
    {{0, -1}, {1  , 0}, {0, 1 }, {-1, 0}}
};

void SpaceCraft::logDecision(const std::string& decision) {
    bool consol = true;
    if (consol){
        cout << "Decision: " << decision << endl;
    }
    else {
    fstream logFile("../Test/Log.txt", ios::out | ios::app); 

    if (!logFile.is_open()) {
        throw runtime_error("Couldn't Open the log file");
    }

    logFile << decision << endl;

    logFile.close();
    }
}

void SpaceCraft::consumeEnergy(int amount){
    energy -= amount;
}

bool SpaceCraft::enoughEnergy(int amount) const {
    return energy >= amount;
}

void SpaceCraft::moveCraft(Map* currentMap) {
    
    Cardinal destination = currentMap->destination;

    vector<vector<bool>> visited(currentMap->size.heightMap, vector<bool>(currentMap->size.weightMap, false));
    if (backtrack(position, visited, currentMap, position)) {
        logDecision("Arrived at destination (" + to_string(destination.x) + ", " + to_string(destination.y) + ")");
    } else {
        logDecision("Failed to reach destination");
    }
}

bool SpaceCraft::backtrack(Cardinal current, vector<vector<bool>>& visited, Map* currentMap, Cardinal pervious) {
    if (currentMap->getCellType(current.x, current.y) == '5') {
        position = current; 
        return true;
    }

   
    visited[current.x][current.y] = true;
    position = current; 

    char typeCurrent = currentMap->getCellType(current.x, current.y);
    int energyCost   = 0;

    switch (typeCurrent)
    {
        case '4':
            if (enoughEnergy(energy/2)){    
                logDecision("Teleporting from (" + to_string(current.x) + ", " + to_string(current.y) + ")");
                consumeEnergy(energy / 2);
                current = teleport(currentMap->wormhole, current); 
                logDecision("Teleporting to (" + to_string(current.x) + ", " + to_string(current.y) + ")");
                position = current;
            }else{
                logDecision("out of energy! (Warmhole)");
                return false;
            }
            break;

        case '3':
            if (enoughEnergy(3*4)){
                logDecision("Orbiting from (" + to_string(pervious.x) + ", " + to_string(pervious.y) + ")");
                consumeEnergy(3*4);
                current = orbit(currentMap->spaceObject, pervious, current);
                logDecision("Orbiting to (" + to_string(current.x) + ", " + to_string(current.y) + ")");
            }else {
                logDecision("out of energy (Orbit)");
                return false;
            }
            break;
        case '1':
            energyCost = spaceCurrentLengthFactor * 2;
            if (enoughEnergy(energyCost)){    
                logDecision("Riding from (" + to_string(current.x) + ", " + to_string(current.y) + ")");
                consumeEnergy(energyCost);
                current = ride(currentMap->spaceCurrent, current);
                logDecision("Riding to (" + to_string(current.x) + ", " + to_string(current.y) + ")");
                position = current;
            }else {
                logDecision("out of energy (Space Current)");
                return false;
            }    
            break;  
        default :
            if (enoughEnergy(1)){
                consumeEnergy(1);
                logDecision("Moving form (" + to_string(current.x) + ", " + to_string(current.y) + ")");
            }else {
                logDecision("out of energy (Move)");
                return false;
            }
            break;
    }

    // const vector<pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    
    for (int i = 0; i < allDirections.size(); ++i){
        currentDirection = i;
        const vector<pair<int,int>>& directions = allDirections[currentDirection];

        int initialEnergy = energy;
        Cardinal initialStartLoc = position;

        for (const auto& direction : directions) {                  
            Cardinal nextMove = {current.x + direction.first, current.y + direction.second};

            nextMove = decision(currentMap, current, nextMove);
            
            
            if(isValidPosition(nextMove.x,nextMove.y,currentMap) && !visited[nextMove.x][nextMove.y]){    
                if(enoughEnergy(1)){
                logDecision("Moving to (" + to_string(nextMove.x) + ", " + to_string(nextMove.y) + ")");
                if(backtrack(nextMove, visited, currentMap, current)){return true;}
                    logDecision("Backtracking from (" + to_string(nextMove.x) + ", " + to_string(nextMove.y) + ")");
                }else{
                    logDecision("out of energy (move)");
                }
            }
        }

        // energy = initialEnergy;
        // position = initialStartLoc;
        // cout << "---------------------------------------------------------------------------------\n";
        // cout << "Directions for currentDirection " << currentDirection << ":\n";
        // for (const auto& dir : directions) {
        //     cout << "(" << dir.first << ", " << dir.second << ")\n";
        // }
        // cout << "---------------------------------------------------------------------------------\n";
    }

    visited[current.x][current.y] = false;
    return false;
}




bool SpaceCraft::isValidPosition(int x, int y, Map* currentMap) {
    return x >= 0 && x < currentMap->size.heightMap && y >= 0 
        && y < currentMap->size.weightMap
        && currentMap->getCellType(x, y) != '2';
}

Cardinal SpaceCraft::decision(Map* currentMap, Cardinal current, Cardinal nextMove) {
    const vector<pair<int, int>> directions = {
        {-1, -1},  {-1, 0}, {-1, 1}, 
        {0, -1 },/*current*/{ 0, 1}, 
        {1, -1 },  { 1, 0}, { 1, 1}
    };

    for (const auto& direction : directions) {
        int x = current.x + direction.first;
        int y = current.y + direction.second;

        if (isValidPosition(x, y, currentMap)) {
            char viewCellType = currentMap->getCellType(x, y);

            if (viewCellType == '5') {
                if (x == current.x || y == current.y) { return {x, y}; }
                else{
                    char xCellType = currentMap->getCellType(current.x, y);
                    char yCellType = currentMap->getCellType(x, current.y);

                    if (!(xCellType == '4' || xCellType == '3' || xCellType == '1')) { return {current.x, y}; }
                    else if (!(yCellType == '4' || yCellType == '3' || yCellType == '1')) { return {x, current.y}; }
                    else{ return nextMove; }
                
                }
            }
        }
    }
    return nextMove;
}

Cardinal SpaceCraft::orbit(SpaceObject so, Cardinal start, Cardinal pos)
{
    
    // create bound of object
    int x1 = so.pos1.x, x2 = so.pos4.x,
        y1 = so.pos1.y, y2 = so.pos4.y; 

    // routing
    if (start.x < x1) { start.x += 3; }
    else if (start.x > x2) { start.x -= 3; }
    else if(start.y < y1) { start.y += 3; }
    else if(start.y > y2) { start.y -= 3; }
    
    return start;
}

Cardinal SpaceCraft::teleport(Wormhole wh, Cardinal start)
{
    if(start.x == wh.pos1.x && start.y == wh.pos1.y) { return wh.pos2; } 
    else return wh.pos1;
}

Cardinal SpaceCraft::ride(vector<SpaceCurrent> sc, Cardinal start)
{
    for (const SpaceCurrent &spaceCurrent : sc)
    {   
        if ((start.x == spaceCurrent.pos1.x && start.y == spaceCurrent.pos1.y) ||
            (start.x == spaceCurrent.pos2.x && start.y == spaceCurrent.pos2.y)) {
            return (start.x == spaceCurrent.pos1.x && start.y == spaceCurrent.pos1.y) ? spaceCurrent.pos2 : spaceCurrent.pos1;
        } 
    }
    return start;
    
}