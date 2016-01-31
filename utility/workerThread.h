//Copyright (c) 2014,
//Kadir Erdem Demir
//All rights reserved.

//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. All advertising materials mentioning features or use of this software
//   must display the following acknowledgement:
//   This product includes software developed by the <organization>.
//4. Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY Kadir Erdem Demir ''AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#define ASIO_STANDALONE

#include <asio.hpp>
#include <future>
#include <memory>



class Worker
{
public:
    Worker() : m_work(m_io_service)
    {
        m_threadCount = 4;
        initThreads();
    }

    void initThreads()
    {
        for( size_t x = 0; x < m_threadCount; ++x )
        {
            std::thread tempThread(std::bind( &Worker::workerThread, this));
            m_threadGroup.push_back(std::move(tempThread));
        }
    }

    void workerThread()
    {
        m_io_service.run();
    }

    void post ( std::shared_ptr< std::packaged_task< void() > > task )
    {
        m_packagedTaskVec.push_back(task);
    }

    void process ()
    {
        for (auto elem : m_packagedTaskVec)
        {
            m_io_service.post([elem](){
                (*elem)();
            });
        }
            //m_io_service.post(std::bind(&std::packaged_task<void()>::operator(), elem.get()));
        for(auto elem : m_packagedTaskVec)
            elem->get_future().wait();
    }

    void clearTasks()
    {
        //m_packagedTaskVec.clear();
        for(auto elem : m_packagedTaskVec)
            elem->reset();
    }

private:
    size_t m_threadCount;
    asio::io_service m_io_service;
    asio::io_service::work m_work;
    std::vector< std::shared_ptr<std::packaged_task<void()> >  > m_packagedTaskVec;
    std::vector< std::thread >      m_threadGroup;
};


#endif // WORKERTHREAD_H
