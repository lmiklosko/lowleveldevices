#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <poll.h>
#include <unistd.h>
#include "trace.hpp"
#include <cstring>

template<typename Callback>
class Poller
{
    static constexpr const auto maxFD = 64;
public:
    using storage_type = std::vector<struct pollfd>;
    using callbacks_type = std::map<int, Callback>;
    using readfn_type = std::function<void(int, Callback const&)>;

    explicit Poller(readfn_type readFn) noexcept : readFn(readFn) {}
    ~Poller()
    {
        join();
    }

    void addDescriptor(int fd, Callback cb)
    {
        if (!worker.joinable())
        {
            TRACE("Starting worker. Adding FD: ", fd);
            std::unique_lock lock(mutex);

            /* Even if the worker starts before we add relevant items to the storage, it still needs
             * to wait for the mutex lock */
            worker = std::thread(&Poller::threadFunc, this);

            pipe(_fd);
            _storage.reserve(maxFD);
            _storage.push_back({_fd[0], POLLPRI | POLLIN, 0});
            _storage.push_back({fd, POLLPRI | POLLERR, 0});
            _callbacks[fd] = cb;

            /* Wait for the thread to be constructed. This is necessary for example when we have two
             * or more consecutive calls to addDescriptor. In the case the thread has not been able
             * to start running yet, we would encounter a deadlock when main thread requests the lock,
             * but worker hasn't been started yet, so it successfully acquires the lock. Worker reads
             * cached bytes in pipe, releasing the lock and waiting for the notification, which does not arrive. */

            /* We also cannot use cv.wait here as it might happen that the thread called notify_all
             * before we reach this line */
            lock.unlock();
            while (lock.try_lock()) lock.unlock();
        }
        else
        {
            executeInContext([fd, cb](storage_type& storage, callbacks_type& callbacks){
                auto it = std::find_if(storage.begin(), storage.end(),
                                       [fd](struct pollfd i){ return fd == i.fd; });
                if (it == storage.end())
                {
                    storage.push_back({fd, POLLPRI | POLLIN, 0});
                    TRACE("Adding descriptor ", fd);
                }
                callbacks[fd] = cb;
            });
        }
    }

    void removeDescriptor(int fd)
    {
        executeInContext([fd](storage_type& storage, callbacks_type& callbacks){
            if (auto it = std::find_if(storage.begin(), storage.end(), [fd](struct pollfd i){
                    return i.fd == fd;
                }); it != storage.end())
            {
                /* mark file descriptor for deletion.
                 * This is required if we call it from the callback as erasing elements from vector would discard
                 * the iterators. Map does not have such constraints */

                callbacks.erase(it->fd);
                it->fd = -1;
            }
        });

        TRACE("Removed descriptor ", fd);
        if (_storage.size() == 1)
        {
            join();
        }
    }

    template<typename Callable>
    void executeInContext(Callable&& func)
    {
        if (isCalledFromCallback())
        {
            TRACE("executeInContext called from the callback");
            std::invoke(func,
                        std::ref(_storage), std::ref(_callbacks));
        }
        else
        {
            TRACE("Requesting worker to stop polling");
            while (::write(_fd[1], "suse\0", 5) < 0) {
                if (errno != EINTR && errno != EIO)
                {
                    TRACE("::write error: ", errno, " - ", strerror(errno));
                    break;
                }
            }

            {
                std::unique_lock lock(mutex);
                std::invoke(func,
                            std::ref(_storage), std::ref(_callbacks));
            }
            cv.notify_all();
        }
    }

protected:
    [[nodiscard]] inline bool isCalledFromCallback() const noexcept
    {
        return std::this_thread::get_id() == worker.get_id();
    }

    void join()
    {
        if (worker.joinable())
        {
            /* Close writing end of the pipe. Worker receives POLLHUP event indicating
             * we want to terminate the thread */
            close(_fd[1]);

            /* Join and close the reading end of a pipe */
            if (worker.joinable())
            {
                worker.join();
            }
            close(_fd[0]);

            /* Clear storage/callbacks in case we'll restart the worker again */
            _storage.clear();
            _callbacks.clear();
        }
    }

    void threadFunc()
    {
        /* Acquire the lock for ourselves */
        std::unique_lock lock(mutex);
        while (true)
        {
            /* Remove all discarded FDs before call to poll */
            std::remove_if(_storage.begin(), _storage.end(), [](struct pollfd const& i){
                return i.fd == -1;
            });

            if (int rv = poll(_storage.data(), _storage.size(), -1); rv > 0)
            {
                /* Pipe sync request */
                if (_storage.begin()->revents)
                {
                    if (_storage.begin()->revents & POLLHUP)
                    {
                        TRACE("Accepting terminate request");
                        break;
                    }

                    char buff[0x10];
                    while (::read(_fd[0], buff, sizeof(buff)) < 0) {
                        if (errno != EINTR && errno != EIO)
                        {
                            TRACE("::write error: ", errno, " - ", strerror(errno));
                            break;
                        }
                    }

                    TRACE("Giving control of storage away");

                    /* Wait for main thread to execute its stuff */
                    cv.wait(lock);

                    TRACE("Restarting poll");
                    continue;
                }

                for (auto pfd = _storage.begin() + 1; pfd != _storage.end(); ++pfd)
                {
                    if (pfd->revents)
                    {
                        try
                        {
                            std::invoke(readFn,
                                        pfd->fd, _callbacks.at(pfd->fd));
                        }
                        catch(std::out_of_range const&)
                        {
                            TRACE("Trying to read fd ", pfd->fd, " without an active callback set.");
                        }
                    }
                }
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            { /* Debug trace breakpoint */ }
        }
    }

private:
    std::thread worker{};
    std::vector<struct pollfd> _storage{};
    std::map<int, Callback> _callbacks{};
    std::condition_variable cv{};
    std::mutex mutex{};
    int _fd[2]{};

    readfn_type readFn;
};