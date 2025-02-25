/* ************************************************************************
 * Copyright (C) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell cop-
 * ies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM-
 * PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNE-
 * CTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * ************************************************************************ */

#include "rocblas_data.hpp"
#include "rocblas_datatype2string.hpp"
#include "rocblas_test.hpp"
#include "testing_ger.hpp"
#include "testing_ger_batched.hpp"
#include "testing_ger_strided_batched.hpp"
#include "type_dispatch.hpp"
#include <cstring>
#include <type_traits>

namespace
{
    // possible ger test cases
    enum ger_test_type
    {
        GER,
        GER_BATCHED,
        GER_STRIDED_BATCHED,
    };

    //ger test template
    template <template <typename...> class FILTER, ger_test_type GER_TYPE>
    struct ger_template : RocBLAS_Test<ger_template<FILTER, GER_TYPE>, FILTER>
    {
        // Filter for which types apply to this suite
        static bool type_filter(const Arguments& arg)
        {
            return rocblas_simple_dispatch<ger_template::template type_filter_functor>(arg);
        }

        // Filter for which functions apply to this suite
        static bool function_filter(const Arguments& arg)
        {
            switch(GER_TYPE)
            {
            case GER:
                return !strcmp(arg.function, "ger") || !strcmp(arg.function, "ger_bad_arg");
            case GER_BATCHED:
                return !strcmp(arg.function, "ger_batched")
                       || !strcmp(arg.function, "ger_batched_bad_arg");
            case GER_STRIDED_BATCHED:
                return !strcmp(arg.function, "ger_strided_batched")
                       || !strcmp(arg.function, "ger_strided_batched_bad_arg");
            }
            return false;
        }

        // Google Test name suffix based on parameters
        static std::string name_suffix(const Arguments& arg)
        {
            RocBLAS_TestName<ger_template> name(arg.name);

            name << rocblas_datatype2string(arg.a_type);

            if(strstr(arg.function, "_bad_arg") != nullptr)
            {
                name << "_bad_arg";
            }
            else
            {
                name << '_' << arg.M << '_' << arg.N << '_' << arg.alpha << '_' << arg.incx;

                if(GER_TYPE == GER_STRIDED_BATCHED)
                    name << '_' << arg.stride_x;

                name << '_' << arg.incy;

                if(GER_TYPE == GER_STRIDED_BATCHED)
                    name << '_' << arg.stride_y;

                name << '_' << arg.lda;

                if(GER_TYPE == GER_STRIDED_BATCHED)
                    name << '_' << arg.stride_a;

                if(GER_TYPE == GER_STRIDED_BATCHED || GER_TYPE == GER_BATCHED)
                    name << '_' << arg.batch_count;
            }

            if(arg.api == FORTRAN)
            {
                name << "_F";
            }

            return std::move(name);
        }
    };

    // By default, this test does not apply to any types.
    // The unnamed second parameter is used for enable_if_t below.
    template <typename, typename = void>
    struct ger_testing : rocblas_test_invalid
    {
    };

    // When the condition in the second argument is satisfied, the type combination
    // is valid. When the condition is false, this specialization does not apply.
    template <typename T>
    struct ger_testing<T, std::enable_if_t<std::is_same_v<T, float> || std::is_same_v<T, double>>>
        : rocblas_test_valid
    {
        void operator()(const Arguments& arg)
        {
            if(!strcmp(arg.function, "ger"))
                testing_ger<T, false>(arg);
            else if(!strcmp(arg.function, "ger_bad_arg"))
                testing_ger_bad_arg<T, false>(arg);
            else if(!strcmp(arg.function, "ger_batched"))
                testing_ger_batched<T, false>(arg);
            else if(!strcmp(arg.function, "ger_batched_bad_arg"))
                testing_ger_batched_bad_arg<T, false>(arg);
            else if(!strcmp(arg.function, "ger_strided_batched"))
                testing_ger_strided_batched<T, false>(arg);
            else if(!strcmp(arg.function, "ger_strided_batched_bad_arg"))
                testing_ger_strided_batched_bad_arg<T, false>(arg);
            else
                FAIL() << "Internal error: Test called with unknown function: " << arg.function;
        }
    };

    using ger = ger_template<ger_testing, GER>;
    TEST_P(ger, blas2)
    {
        CATCH_SIGNALS_AND_EXCEPTIONS_AS_FAILURES(rocblas_simple_dispatch<ger_testing>(GetParam()));
    }
    INSTANTIATE_TEST_CATEGORIES(ger);

    using ger_batched = ger_template<ger_testing, GER_BATCHED>;
    TEST_P(ger_batched, blas2)
    {
        CATCH_SIGNALS_AND_EXCEPTIONS_AS_FAILURES(rocblas_simple_dispatch<ger_testing>(GetParam()));
    }
    INSTANTIATE_TEST_CATEGORIES(ger_batched);

    using ger_strided_batched = ger_template<ger_testing, GER_STRIDED_BATCHED>;
    TEST_P(ger_strided_batched, blas2)
    {
        CATCH_SIGNALS_AND_EXCEPTIONS_AS_FAILURES(rocblas_simple_dispatch<ger_testing>(GetParam()));
    }
    INSTANTIATE_TEST_CATEGORIES(ger_strided_batched);

} // namespace
