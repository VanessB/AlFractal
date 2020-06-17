#ifndef ALFRACTAL_ALGEBRA
#define ALFRACTAL_ALGEBRA

#include <valarray>

namespace algebra
{
    ////////////////     Algebra     ///////////////
    // Реализация алгебры над полем.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    class Algebra
    {
    public:
        std::valarray<field> components;

        explicit Algebra<field, dimension, product_tensor>()
            : components(field{0}, dimension)
        { }
        explicit Algebra<field, dimension, product_tensor>(std::valarray<field> initcomponents)
            : components(initcomponents)
        { }

        // Нахождение противоположного элемента.
        Algebra<field, dimension, product_tensor> operator-(const Algebra<field, dimension, product_tensor>& right)
        {
            Algebra<field, dimension, product_tensor> result = right;
            result.components = -result.components;
            return result;
        }

        // Сложение с элементом алгебры.
        Algebra<field, dimension, product_tensor>& operator+=(const Algebra<field, dimension, product_tensor>& right)
        { components += right.components; return *this; }
        Algebra<field, dimension, product_tensor>& operator-=(const Algebra<field, dimension, product_tensor>& right)
        { components -= right.components; return *this; }

        // Умножение и деление на элемент поля.
        Algebra<field, dimension, product_tensor>& operator*=(const field& right)
        { components *= right; return *this; }
        Algebra<field, dimension, product_tensor>& operator/=(const field& right)
        { components /= right; return *this; }

        // Умножение на элемент алгебры.
        Algebra<field, dimension, product_tensor>& operator*=(const Algebra<field, dimension, product_tensor>& right)
        {
            // Одномерное представление массива тензора произведения.
            const field (&_product_tensor)[dimension * dimension * dimension] = reinterpret_cast<const field (&)[dimension * dimension * dimension]>(product_tensor);

            // Новые компоненты элемента алгебры.
            std::valarray<field> new_components = std::valarray<field>(field{0}, dimension);

            // Цикл по базисным элементам.
            for (size_t index = 0; index < dimension; ++index)
            {
                field component{0}; // Временная переменная для компонента на позиции index.
                size_t component_offset = index * dimension * dimension; // Смещение в одномерном массиве при доступе к компоненте с индексом index.

                // Цикл по строкам среза массива.
                for (size_t row = 0; row < dimension; ++row)
                {
                    field sum{0}; // Временная переменная для произведения правого столбца компонент на строку среза.
                    size_t row_offset = row * dimension + component_offset;

                    // Произведение правого столбца компонент на строку среза.
                    for (size_t column = 0; column < dimension; ++column)
                    { sum += _product_tensor[row_offset + column] * right.components[column]; }

                    // Умножение на компоненту левого столбца компонент.
                    component += sum * components[row];
                }
                new_components[index] = component;
            }

            components = new_components;
            return *this;
        }

    protected:

    private:

    };

    // Сумма элементов векторного пространства.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator+(const Algebra<field, dimension, product_tensor>& left, const Algebra<field, dimension, product_tensor>& right)
    {
        Algebra<field, dimension, product_tensor> result = left;
        result += right;
        return result;
    }

    // Разность элементов векторного пространства.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator-(const Algebra<field, dimension, product_tensor>& left, const Algebra<field, dimension, product_tensor>& right)
    {
        Algebra<field, dimension, product_tensor> result = left;
        result -= right;
        return result;
    }

    // Произведение элемента векторного пространства и элемента поля.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator*(const Algebra<field, dimension, product_tensor>& left, const field& right)
    {
        Algebra<field, dimension, product_tensor> result = left;
        result *= right;
        return result;
    }
    template <class field, size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator*(const field& left, const Algebra<field, dimension, product_tensor>& right)
    {
        return right * left;
    }

    // Частное элемента векторного пространства и элемента поля.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator/(const Algebra<field, dimension, product_tensor>& left, const field& right)
    {
        Algebra<field, dimension, product_tensor> result = left;
        result /= right;
        return result;
    }

    // Произведение элементов алгебры.
    template <class field, const size_t dimension, const field (&product_tensor)[dimension][dimension][dimension]>
    Algebra<field, dimension, product_tensor> operator*(const Algebra<field, dimension, product_tensor>& left, const Algebra<field, dimension, product_tensor>& right)
    {
        Algebra<field, dimension, product_tensor> result = left;
        result *= right;
        return result;
    }


    ////////////////     Complex     ///////////////
    // Комплексные числа.
    const double complex_pt[2][2][2]
    {
        {
            // Real.
            // 1     i
            { 1.0,  0.0 }, // 1
            { 0.0, -1.0 }  // i
        },
        {
            // Imaginary.
            // 1    i
            { 0.0, 1.0 }, // 1
            { 1.0, 0.0 }  // i
        }
    };
    using Complex = Algebra<double, 2, complex_pt>;


    ////////////////  SplitComplex  ////////////////
    // Двойные числа.
    const double split_complex_pt[2][2][2]
    {
        {
            // xi_1
            // 1     2
            { 1.0, 0.0 }, // 1
            { 0.0, 0.0 }  // 2
        },
        {
            // xi_2
            // 1    2
            { 0.0, 0.0 }, // 1
            { 0.0, 1.0 }  // 2
        }
    };
    using SplitComplex = Algebra<double, 2, split_complex_pt>;
}

#endif
