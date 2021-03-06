#include "nMessageMock.h"
#include "tPolynomial.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

class tPolynomialTest : public CppUnit::TestFixture {
private:
    tPolynomial<nMessageMock> tpEmpty, tpOne, tpZero, tpTwo;

public:
    CPPUNIT_TEST_SUITE( tPolynomialTest );
    CPPUNIT_TEST( testEquality );
    CPPUNIT_TEST( testAddition );
    CPPUNIT_TEST( testMultiplication );
    CPPUNIT_TEST( testEvaluateAndBaseArgument );
    //    CPPUNIT_TEST( testWriteAndReadToStream );
    CPPUNIT_TEST( testParse );
    CPPUNIT_TEST( testTranslate );
    CPPUNIT_TEST( testSubstitute );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        //        tpEmpty = tPolynomial<nMessageMock> ;
        tpZero  = tPolynomial<nMessageMock>(1);
        tpZero[(unsigned)0] = 0.0;
        tpOne   = tPolynomial<nMessageMock>(1);
        tpOne[0] = 1.0;
        tpTwo   = tPolynomial<nMessageMock>(1);
        tpTwo[0] = 2.0;
    }

    void tearDown() {
        // Empty
    }

    void testEquality() {
        CPPUNIT_ASSERT( tpEmpty == tpEmpty );
        CPPUNIT_ASSERT( tpOne == tpOne );
        CPPUNIT_ASSERT( tpEmpty == tpZero );
        CPPUNIT_ASSERT( !(tpZero == tpOne) );
        // testing the copy constructor
        CPPUNIT_ASSERT( tpOne == tPolynomial<nMessageMock>(tpOne) );

        // testing the tArray constructor
        float values[] = {1, 3, 5, 7};
        int size = sizeof(values)/sizeof(values[0]);
        tArray<float> tValues(size);
        for (int i=0; i<size; i++) {
            tValues[i] = values[i];
        }
        tPolynomial<nMessageMock> tpByArray(values, size);
        tPolynomial<nMessageMock> tpBytArray(tValues);

        CPPUNIT_ASSERT(tpByArray == tpBytArray);

        // Testing the assignment operator
        float randomData[] = {3, 5, 7};
        tPolynomial<nMessageMock> initiated(randomData, sizeof(randomData)/sizeof(randomData[0]));
        tPolynomial<nMessageMock> uninitiated;

        initiated = tpTwo;
        uninitiated = tpTwo;
        CPPUNIT_ASSERT( 2.0 == tpTwo[0] );
        CPPUNIT_ASSERT( 2.0 == initiated[0] );
        CPPUNIT_ASSERT( tpTwo == initiated );
        CPPUNIT_ASSERT( tpTwo == uninitiated );
    }

    void testAddition() {
        // {0} = {-} + {0}
        CPPUNIT_ASSERT( tpZero == tpEmpty + tpZero );
        // {1} = {0} + {1}
        CPPUNIT_ASSERT( tpOne == tpZero + tpOne );
        // {2} = {1} + {1}
        CPPUNIT_ASSERT( tpTwo == tpOne + tpOne );
        // a = b + c
        float a[] = {3, 5, 7, 11};
        float b[] = {0, 5, 3, -5};
        float c[] = {3, 0, 4, 16};
        tPolynomial<nMessageMock> tpA(a, sizeof(a)/sizeof(a[0]));
        tPolynomial<nMessageMock> tpB(b, sizeof(b)/sizeof(b[0]));
        tPolynomial<nMessageMock> tpC(c, sizeof(c)/sizeof(c[0]));

        CPPUNIT_ASSERT( tpA == tpB + tpC );

        // {0} = {-} + 0
        CPPUNIT_ASSERT( tpZero == tpEmpty + 0.0 );
        // {1} = {0} + 1
        CPPUNIT_ASSERT( tpOne == tpZero + 1.0 );
        // {2} = {1} + 1
        CPPUNIT_ASSERT( tpTwo == tpOne + 1.0 );

        // {a} + 1 == ({b} + 2) + {c} + -1
        CPPUNIT_ASSERT( (tpA + 1.0) == (tpB + 2.0) + tpC + -1.0);


        tPolynomial<nMessageMock> tpAat5 = tpA.adaptToNewReferenceVarValue(5.0);
        tPolynomial<nMessageMock> tpBat5 = tpB.adaptToNewReferenceVarValue(5.0);
        tPolynomial<nMessageMock> tpCat5 = tpC.adaptToNewReferenceVarValue(5.0);
        CPPUNIT_ASSERT( tpAat5 == tpBat5 + tpCat5 );

        //
        // Adding 2 polynomial having different referenceVarValue
        //
        tPolynomial<nMessageMock> tpAat3 = tpA.adaptToNewReferenceVarValue(3.0);
        tPolynomial<nMessageMock> tpCat7 = tpC.adaptToNewReferenceVarValue(7.0);

        // Addition
        tPolynomial<nMessageMock> tpAat3at5 = tpAat3.adaptToNewReferenceVarValue(5.0);
        tPolynomial<nMessageMock> tpCat7at5 = tpCat7.adaptToNewReferenceVarValue(5.0);
        tPolynomial<nMessageMock> sum = (tpBat5 + tpCat7);
        CPPUNIT_ASSERT( tpAat3 == tpBat5 + tpCat7 );

        // Addition with operator+=
        // This modified tpBat5
        CPPUNIT_ASSERT( tpAat3 == (tpBat5 += tpCat7) );
    }

    void testMultiplication() {
        // {0} = {-} * {0}
        CPPUNIT_ASSERT( tpZero == tpEmpty * tpZero );
        // {0} = {0} * {1}
        CPPUNIT_ASSERT( tpZero == tpZero * tpOne );
        // {1} = {1} * {1}
        CPPUNIT_ASSERT( tpOne == tpOne * tpOne );
        // {1,0,0} = {1,0} * {1,0}
        float onePower2[] = {0, 1};
        tPolynomial<nMessageMock> tpOnePower2( onePower2, sizeof(onePower2)/sizeof(onePower2[0]) );
        float onePower3[] = {0, 0, 1};
        tPolynomial<nMessageMock> tpOnePower3( onePower3, sizeof(onePower3)/sizeof(onePower3[0]) );

        CPPUNIT_ASSERT( tpOnePower3 == (tpOnePower2 * tpOnePower2) );

        {
            // a = b * c
            float a[] = {3.0, 2.5, -6.0, -17, -5, 12, 12};
            float b[] = {1, 0.0, -2, -2};
            float c[] = {3, 2.5, 0, -6};
            tPolynomial<nMessageMock> tpA(a, sizeof(a)/sizeof(a[0]));
            tPolynomial<nMessageMock> tpB(b, sizeof(b)/sizeof(b[0]));
            tPolynomial<nMessageMock> tpC(c, sizeof(c)/sizeof(c[0]));

            CPPUNIT_ASSERT( tpA == tpB * tpC );

            // {0} = {-} * 0
            CPPUNIT_ASSERT( tpZero == tpEmpty * 0.0 );
            // {0} = {0} * 1
            CPPUNIT_ASSERT( tpZero == tpZero * 1.0 );
            // {1} = {1} * 1
            CPPUNIT_ASSERT( tpOne == tpOne * 1.0 );
            // {a} * -2 == ({b} * 2) * ({c} * -1)
            CPPUNIT_ASSERT( (tpA * -2.0) == (tpB * 2.0) * (tpC * -1.0));
        }

        // Can a tPolynomial be used to make a zone shape turn?
        // Zone rotation are described by 4 term:
        // a : basic orientation angle
        // b : orientation angle in function of the conquest state
        // c : basic rotation speed
        // d : rotation speed in function of the conquest state
        //
        // The polynomial used to describe the actual rotation gets computed from both
        // a + conquestRate * b + t * (c + conquestRate * d)
        //
        {
            float a = 1.0;
            float b = 2.0;
            float c = 3.0;
            float d = 4.0;
            float conquestState[] = {3, 5, 7};
            // Manually resolving the following:
            // a + conquestRate * b + t * (c + conquestRate * d)
            // 1 + {3, 5, 7} * 2 + t * ( 3 + {3, 5, 7} * 4 }
            // 1 + {6, 10, 14} + t * ( 3 + {12, 20, 28} }
            // {7, 10, 14} + t * ( {15, 20, 28} }
            // {7, 10, 14} + {0, 15, 20, 28}
            // {7, 25, 34, 28}
            //
            float resValue[] = {7, 25, 34, 28};
            tPolynomial<nMessageMock> res(resValue, sizeof(resValue)/sizeof(resValue[0]));

            float t[] = {0.0, 1.0};
            tPolynomial<nMessageMock> tpT( t, sizeof(t)/sizeof(t[0]) );
            tPolynomial<nMessageMock> tpConquestState( conquestState, sizeof(conquestState)/sizeof(conquestState[0])  );

            tPolynomial<nMessageMock> tf =
                ( (tpConquestState * b) + a)
                + tpT * ( (tpConquestState * d) + c);

            CPPUNIT_ASSERT(res == tf);
        }
    }

#define DELTA 1e-3

    void testEvaluateAndBaseArgument() {
        float accelA = 10.0;
        float accelB = -10.0;
        float a[] = {0, 0, accelA};
        float b[] = {50, 45, accelB};
        tPolynomial<nMessageMock> tfA(a, sizeof(a)/sizeof(a[0]));
        tPolynomial<nMessageMock> tfB(b, sizeof(b)/sizeof(b[0]));

        CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, tfA.evaluate(0), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 125.0, tfA.evaluate(5), DELTA);

        CPPUNIT_ASSERT_DOUBLES_EQUAL( 50.0, tfB.evaluate(0), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, tfB.evaluate(10), DELTA);

        // change the base argument and adjust the coefs to time 5
        float NEW_REFERENCE_TIME = 5.0;
        tfA = tfA.adaptToNewReferenceVarValue(NEW_REFERENCE_TIME);
        tfB = tfB.adaptToNewReferenceVarValue(NEW_REFERENCE_TIME);

        // These should still be true
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, tfA.evaluate(0), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 125.0, tfA.evaluate(5), DELTA);

        CPPUNIT_ASSERT_DOUBLES_EQUAL( 50.0, tfB.evaluate(0), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, tfB.evaluate(10), DELTA);

        // changing the acceleration
        tfA.changeRate(2.0*accelA, 2, NEW_REFERENCE_TIME);
        tfB.changeRate(2.0*accelB, 2, NEW_REFERENCE_TIME);

        CPPUNIT_ASSERT_DOUBLES_EQUAL( 125.0 + 50*0 + 20/2.0*0*0, tfA.evaluate(5), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 125.0 + 50*5 + 20/2.0*5*5, tfA.evaluate(10), DELTA);

        CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.0 - 5*0 - 20.0/2.0*0*0, tfB.evaluate(5), DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.0 - 5*5 - 20.0/2.0*5*5, tfB.evaluate(10), DELTA);
    }

    /*
      void testWriteAndReadToStream() {
          float a[] = {1.0, 2.0, 10.0};
          float b[] = {50, 45, -10.0};
          tPolynomial<nMessageMock> tfA(a, sizeof(a)/sizeof(a[0]));
          tPolynomial<nMessageMock> tfB(b, sizeof(b)/sizeof(b[0]));
          tPolynomial<nMessageMock> tfC;

          nMessageMock messageMock;

          CPPUNIT_ASSERT( !(tfA == tfB) );

          messageMock << tfA;
          messageMock.receive();
          messageMock >> tfB;

          CPPUNIT_ASSERT( tfA == tfB );
      }
    */

  /*! \brief validate the parsing mechanism
   *
   * Validate the parsing mechanism by loading different strings.
   */
  void testParse() {
    tPolynomial<nMessageMock> tp;

    // Happy sunshine flow
    {
      tp.parse("");
      CPPUNIT_ASSERT( tpEmpty == tp );
      
      tp.parse("2");
      CPPUNIT_ASSERT( tpTwo == tp );
      
      tp.parse("3;5;7;11");
      float b[] = {3, 5, 7, 11};
      tPolynomial<nMessageMock> tpB(b, sizeof(b)/sizeof(b[0]));
      CPPUNIT_ASSERT( tpB == tp );
      
      // Try with a smaller polynomial now, ensuring that there is no leakage
      tp.parse("3;5");
      float a[] = {3, 5};
      tPolynomial<nMessageMock> tpA(a, sizeof(a)/sizeof(a[0]));
      CPPUNIT_ASSERT( tpA == tp );

      // Quick notation
      tp.parse(";;3;5");
      float c[] = {0, 0, 3, 5};
      tPolynomial<nMessageMock> tpC(c, sizeof(c)/sizeof(c[0]));
      CPPUNIT_ASSERT( tpC == tp );

    }

    // not the happy sunshine flow
    {
      // no elements should be created
      tp.parse(";;;;;");
      CPPUNIT_ASSERT( tpEmpty == tp );
      
      // quick notation with extra delimiters
      tp.parse(";;3;5;;");
      float a[] = {0, 0, 3, 5};
      tPolynomial<nMessageMock> tpA(a, sizeof(a)/sizeof(a[0]));
      CPPUNIT_ASSERT( tpA == tp );

      // non-numerical information
      tp.parse("Hello world!;0;");
      CPPUNIT_ASSERT( tpEmpty == tp );
    }
  }


  void testTranslate() {
    tPolynomial<nMessageMock> tpSource("0;0;1");
    tPolynomial<nMessageMock> tpRes("9;6;1");
    std::cout << std::endl;
    std::cout << "translate" << std::endl;
    std::cout << tpSource.toString() << std::endl;
    std::cout << tpSource.translate(3).toString() << std::endl;
    std::cout << tpRes.toString() << std::endl;
    CPPUNIT_ASSERT( tpRes == tpSource.translate(3) );
  }

  void testSubstitute() {
    // This test will use the following notation:
    // tpResultat == tpTransformer.substitute(tpValue);
    tPolynomial<nMessageMock> tpValue;
    tPolynomial<nMessageMock> tpTransformer;
    tPolynomial<nMessageMock> tpResultat;


    // Similar size
    tpValue.parse("3;5");
    tpTransformer.parse("7;9");
    // V(x) = {3, 5}
    // T(y) = {7, 9}
    // T(V(x)) :
    // a + b*y
    // 7 + 9*y
    // 7 + 9*(3 + 5*x)
    // 7 + 27 + 45*x
    // 34 + 45*x
    tpResultat.parse("34;45");
    CPPUNIT_ASSERT( tpResultat == tpTransformer.substitute(tpValue) );

    // Different size : Much longer value
    tpValue.parse("3;5;7;11;13;17");
    tpTransformer.parse("7;11");
    // V(x) = {3, 5, 7, 11, 13, 17}
    // T(y) = {7, 11}
    // T(V(x)) :
    // a + b*y
    // 7 + 11*y
    // 7 + 11*(3 + 5*x + 7*x^2 + 11*x^3 + 13*x^4 + 17*x^5)
    // 7 + 33 + 55*x + 77*x^2 + 121*x^3 + 143*x^4 + 187*x^5
    // 40 + 55*x + 77*x^2 + 121*x^3 + 143*x^4 + 187*x^5
    tpResultat.parse("40;55;77;121;143;187");
    CPPUNIT_ASSERT( tpResultat == tpTransformer.substitute(tpValue) );

    // Different size : Much longer transformer
    tpValue.parse("3;5");
    tpTransformer.parse("3;5;7;11;13;17");
    // V(x) = {3, 5}
    // T(y) = {3, 5, 7, 11, 13, 17}
    // T(V(x)) :
    // a + b*y + c*y^2 + d*y^3 + e*y^4 + f*y^5
    // 3 + 5*y + 7*y^2 + 11*y^3 + 13*y^4 + 17*y^5
    // 3 + 5*(3 + 5*x) + 7*(3 + 5*x)^2 + 11*(3 + 5*x)^3 + 13*(3 + 5*x)^4 + 17*(3 + 5*x)^5
    // computed using yacas:
    // In> Expand(3 + 5*(3 + 5*x) + 7*(3 + 5*x)^2 + 11*(3 + 5*x)^3 + 13*(3 + 5*x)^4 + 17*(3 + 5*x)^5)
    // Out> 53125*x^5+167500*x^4+212125*x^3+134950*x^2+43165*x+5562
    tpResultat.parse("5562;43165;134950;212125;167500;53125");
    CPPUNIT_ASSERT( tpResultat == tpTransformer.substitute(tpValue) );

    // Substitution on a polynomial which reference point has been moved:
    // Source@t=0 : {0; 1} ;; both are equivalent, only the ref point has been moved
    // Source@t=3 : {3; 1}
    // Transform  : {0;0;1} ;; the transform shouldn't have any ref point
    tPolynomial<nMessageMock> tpSourceA("0;1");
    tPolynomial<nMessageMock> tpSourceB = tpSourceA.adaptToNewReferenceVarValue(3);
    tPolynomial<nMessageMock> tpTransform("0;0;1");

    std::cout << std::endl << "#######"  << std::endl;
    std::cout << tpSourceA.toString() << std::endl;
    std::cout << tpSourceB.toString() << std::endl;
    std::cout << tpTransform.substitute(tpSourceA).toString() << std::endl;
    std::cout << tpTransform.substitute(tpSourceB).toString() << std::endl;
    std::cout << tpTransform.substitute(tpSourceA).adaptToNewReferenceVarValue(3).toString() << std::endl;

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( tPolynomialTest );
