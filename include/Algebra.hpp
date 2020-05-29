#ifndef ALFRACTAL_ALGEBRA
#define ALFRACTAL_ALGEBRA

#include <valarray>

namespace algebra
{
    ////////////////     Algebra     ///////////////
    // Реализация алгебры над полем.
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    class Algebra
    {
    public:
        std::valarray<Field> components;

        explicit Algebra<Field, Dimension, ProductTensor>()
        { components = std::valarray<Field>((Field)( 0 ), Dimension); }
        explicit Algebra<Field, Dimension, ProductTensor>(std::valarray<Field> initcomponents)
        { components = initcomponents; }
        ~Algebra() { }


        // Нахождение противоположного элемента.
        Algebra<Field, Dimension, ProductTensor> operator-(const Algebra<Field, Dimension, ProductTensor>& right)
        {
            Algebra<Field, Dimension, ProductTensor> Result = right;
            Result.components = -Result.components;
            return Result;
        }

        // Сложение с элементом алгебры.
        Algebra<Field, Dimension, ProductTensor>& operator+=(const Algebra<Field, Dimension, ProductTensor>& right)
        { components += right.components; return *this; }
        Algebra<Field, Dimension, ProductTensor>& operator-=(const Algebra<Field, Dimension, ProductTensor>& right)
        { components -= right.components; return *this; }

        // Умножение и деление на элемент поля.
        Algebra<Field, Dimension, ProductTensor>& operator*=(const Field& Right)
        { components *= Right; return *this; }
        Algebra<Field, Dimension, ProductTensor>& operator/=(const Field& Right)
        { components /= Right; return *this; }

        // Умножение на элемент алгебры.
        Algebra<Field, Dimension, ProductTensor>& operator*=(const Algebra<Field, Dimension, ProductTensor>& right)
        {
            // Одномерное представление массива тензора произведения.
            Field* _ProductTensor = (Field*)(ProductTensor);

            // Новые компоненты элемента алгебры.
            std::valarray<Field> new_components = std::valarray<Field>((Field)( 0 ), Dimension );

            // Цикл по базисным элементам.
            for (size_t index = 0; index < Dimension; ++index)
            {
                Field component = (Field)( 0 ); // Временная переменная для компонента на позиции index.
                size_t component_offset = index * Dimension * Dimension; // Смещение в одномерном массиве при доступе к компоненте с индексом index.

                // Цикл по строкам среза массива.
                for (size_t row = 0; row < Dimension; ++row)
                {
                    Field sum = (Field)( 0 ); // Временная переменная для произведения правого столбца компонент на строку среза.
                    size_t row_offset = row * Dimension + component_offset;

                    // Произведение правого столбца компонент на строку среза.
                    for (size_t Column = 0; Column < Dimension; ++Column)
                    { sum += _ProductTensor[row_offset + Column] * right.components[Column]; }

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
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator+(const Algebra<Field, Dimension, ProductTensor>& left, const Algebra<Field, Dimension, ProductTensor>& right)
    {
        Algebra<Field, Dimension, ProductTensor> Result = left;
        Result += right;
        return Result;
    }

    // Разность элементов векторного пространства.
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator-(const Algebra<Field, Dimension, ProductTensor>& left, const Algebra<Field, Dimension, ProductTensor>& right)
    {
        Algebra<Field, Dimension, ProductTensor> Result = left;
        Result -= right;
        return Result;
    }

    // Произведение элемента векторного пространства и элемента поля.
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator*(const Algebra<Field, Dimension, ProductTensor>& left, const Field& right)
    {
        Algebra<Field, Dimension, ProductTensor> Result = left;
        Result *= right;
        return Result;
    }

    // Частное элемента векторного пространства и элемента поля.
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator/(const Algebra<Field, Dimension, ProductTensor>& left, const Field& right)
    {
        Algebra<Field, Dimension, ProductTensor> Result = left;
        Result /= right;
        return Result;
    }
    template <class Field, size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator*(const Field& left, const Algebra<Field, Dimension, ProductTensor>& right)
    {
        return right * left;
    }


    // Произведение элементов алгебры.
    template <class Field, const size_t Dimension, const Field ProductTensor[Dimension][Dimension][Dimension]>
    Algebra<Field, Dimension, ProductTensor> operator*(const Algebra<Field, Dimension, ProductTensor>& left, const Algebra<Field, Dimension, ProductTensor>& right)
    {
        Algebra<Field, Dimension, ProductTensor> Result = left;
        Result *= right;
        return Result;
    }


    ////////////////     Complex     ///////////////
    // Комплексные числа.
    const double ComplexPT[2][2][2] =
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
    using Complex = Algebra<double, 2, ComplexPT>;


    ////////////////  SplitComplex  ////////////////
    // Двойные числа.
    const double SplitComplexPT[2][2][2] =
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
    using SplitComplex = Algebra<double, 2, SplitComplexPT>;
}

#endif
