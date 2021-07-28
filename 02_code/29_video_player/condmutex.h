#ifndef CONDMUTEX_H
#define CONDMUTEX_H

#include <SDL2/SDL.h>

class CondMutex
{
public:
    CondMutex();
    /** 析构函数 */
    ~CondMutex();

    /** 加锁 */
    void lock();
    /** 解锁 */
    void unlock();
    /** 唤醒一个等待_cond条件变量的线程 */
    void signal();
    /** 唤醒所有等待_cond条件变量的线程 */
    void broadcast();
    /** 等待条件变量_cond */
    void wait();

private:
    /** 互斥锁 */
    SDL_mutex *_mutex = nullptr;
    /** 条件变量 */
    SDL_cond *_cond = nullptr;
};

#endif // CONDMUTEX_H
