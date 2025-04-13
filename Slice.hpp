#pragma once

#include <initializer_list>
#include <exception>
#include <optional>
#include <iterator>
#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include <tuple>

#include "Slice_lexer.hpp"

namespace gtd {
    class SliceError: public std::exception {
    public:
        SliceError(const std::string& _msg) : msg{_msg} {}
        const char* what() const noexcept override {
            return msg.c_str(); 
        }
    private:
        std::string msg{};
    };
}

namespace gtd {
    template <typename cT>
    using ciT = typename cT::value_type;
}

namespace gtd {
    template <typename cT>
    class index {
    using T = ciT<cT>;
    public:
        T operator()(const cT& container, std::size_t indx) const {
            return container[indx];
        }
    };
    template <typename cT>
    class size {
    public:
        std::size_t operator()(const cT& container) const {
            return static_cast<std::size_t>(container.size());
        }
    };
    class slice_template {
    public:
        slice_template() {}
        explicit slice_template(int64_t _index) : start{_index}, end{_index+1ll}, step{1ll} {}
        slice_template(std::optional<int64_t> _start, std::optional<int64_t> _end, std::optional<int64_t> _step) 
            : start{_start}, end{_end}, step{_step} {}
        std::optional<int64_t> start{};
        std::optional<int64_t> end{};
        std::optional<int64_t> step{1ll};
        std::tuple<int64_t,int64_t,int64_t> get_from_size(std::size_t size) {
            int64_t _start{}; int64_t _end{}; int64_t _step{};
            if(!step.has_value()) _step = 1ll;
            else                  _step = step.value();
            if(_step == 0ll) throw gtd::SliceError("Slicing error: bad step value");
            if(!start.has_value()) {
                if(_step > 0) _start = 0ll;
                if(_step < 0) _start = size-1ll;
            }
            else if((start.value() > 0 && start.value() >= size))          throw gtd::SliceError("Slicing error: bad start value");
            else if((start.value() < 0 && std::abs(start.value()) > size)) throw gtd::SliceError("Slicing error: bad start value");
            else _start = get_index(start.value(),size);
            if(!end.has_value()) {
                if(_step > 0) _end = size;
                if(_step < 0) _end = -1ll;
            }
            else if((end.value() > 0 && end.value() >= size))          _end = size;
            else if((end.value() < 0 && std::abs(end.value()) > size)) _end = -1ll;
            else                                                       _end = get_index(end.value(),size);
            return std::make_tuple(_start,_end,_step);
        }
    private:
        std::size_t get_index(int64_t indx, std::size_t size) {
            if(indx >= 0ll) return indx;
            else {
                return size + indx;
            }
        }
    };
    template <typename cT>
    class slice {
    public:
    using T = ciT<cT>;
    using rT = std::vector<T>;
        slice(cT& _container) : container{_container} {}
        cT& container{};
        rT operator()(gtd::slice_template templ) {
            rT new_vec{};
            std::size_t size = gtd::size<cT>{}(container);
            auto [start,end,step] = templ.get_from_size(size);
            go( new_vec,
                container.cbegin()+start,container.cbegin()+end,step,
                (step < 0 ? [](cT_c_iter a, cT_c_iter b)->bool{ return a > b; } : [](cT_c_iter a, cT_c_iter b)->bool{ return a < b; })
            );
            return new_vec;
        }
        rT operator()(std::initializer_list<gtd::slice_template> templ_list) {
            rT new_vec{};
            std::size_t size = gtd::size<cT>{}(container);
            for(gtd::slice_template templ : templ_list) {
                auto [start,end,step] = templ.get_from_size(size);
                go( new_vec,
                    container.cbegin()+start,container.cbegin()+end,step,
                    (step < 0 ? [](cT_c_iter a, cT_c_iter b)->bool{ return a > b; } : [](cT_c_iter a, cT_c_iter b)->bool{ return a < b; })
                );
            }
            return new_vec;
        }
        static std::vector<std::size_t> get_indexes(gtd::slice_template templ) {
            std::vector<std::size_t> indx_vec{};
            // Boo~!
            return indx_vec;
        }
        static std::vector<std::size_t> get_indexes(std::initializer_list<gtd::slice_template> templ_list) {
            std::vector<std::size_t> indx_vec{};
            // Boo~!
            return indx_vec;
        }
    protected:
    using cT_c_iter = typename cT::const_iterator;
    using cT_cr_iter = typename cT::const_reverse_iterator;
        std::size_t get_index(int64_t indx) {
            if(indx >= 0ll) return indx;
            else {
                std::size_t size = gtd::size<cT>(container);
                return size + indx;
            }
        }
        std::size_t get_index(int64_t indx, std::size_t size) {
            if(indx >= 0ll) return indx;
            else {
                return size + indx;
            }
        }
        void go(rT& new_vec, cT_c_iter begin, cT_c_iter end, int64_t step, bool(*is_less)(cT_c_iter,cT_c_iter)) {
            for(cT_c_iter iter = begin; is_less(iter,end); iter += step) {
                new_vec.push_back(*iter);
            }
        }
        void go_forwad(rT& new_vec, cT_c_iter begin, cT_c_iter end, std::size_t step) {
            for(cT_c_iter iter = begin; iter < end; iter += step) {
                new_vec.push_back(*iter);
            }
        }
        void go_back(rT& new_vec, cT_cr_iter begin, cT_cr_iter end, std::size_t step) {
            for(cT_cr_iter iter = begin; iter < end; iter += step) {
                new_vec.push_back(*iter);
            }
        }
    };
}

namespace gtd {
    template <typename T>
    class index<std::vector<T>> {
    public:
        T operator()(const std::vector<T>& vec, std::size_t indx) {
            return vec.at(indx);
        }
    };
    template <typename T>
    class size<std::vector<T>> {
    public:
        std::size_t operator()(const std::vector<T>& vec) {
            return vec.size();
        };
    };
}