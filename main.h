#pragma once

void reset_all(bool& stuck, int& movey, double& bgmove, int& moveUni, int& lifes, bool& jump, bool& falling, double& velocityY, bool& unpress, int& jumpT, bool change_control, double& move_velocity, int& frame);

void keyboard_actions(SDL_Event& event, int& quit, int& movey, double& worldTime, double& bgmove, int& moveUni, int& lifes, bool& unpressD, bool& change_control, bool& falling, double& move_velocity, double& velocityY, double& velocityBG, bool blocky, bool stuck, int& jumpT, bool& attack, bool& unpress, bool& jump, int& attackT);

void unicorn_placement(int& movey, int& moveUni);

void unicorn_placementV2(int& moveUni, SDL_Surface* unicorn, bool change_control, SDL_Rect& unic, bool& stuck, double& bgmove, double move_velocity, double worldTime, double velocityBG, int& movey, double& velocityY, bool blocky, bool& falling, int& jumpT, bool attack, int& attackT, bool jump);

void count_fps_time(int& t2, double& delta, int& t1, double& worldTime, bool& change_frame, double& fpsTimer, double& fps, int& frames);

void change_velocity(bool& jump, double& velocityY, bool& falling, bool blocky, bool& attack, double& velocityBG);
