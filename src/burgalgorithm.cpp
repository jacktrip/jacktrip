#include "burgalgorithm.h"
#include <QDebug>
#include <stk/Stk.h>

using namespace std;
using namespace stk;


QString qStringFromLongDouble1(const long double myLongDouble)
{
    std::stringstream ss;
    ss << myLongDouble;

    return QString::fromStdString(ss.str());
}

BurgAlgorithm::BurgAlgorithm()
{

}

// from .pl
void BurgAlgorithm::train(vector<long double> &coeffs, const vector<double> &x )
{
//    qDebug() << "++" << qStringFromLongDouble1(coeffs[1]) << "....." << x[1];

    // GET SIZE FROM INPUT VECTORS
    size_t N = x.size() - 1;
    size_t m = coeffs.size();

    ////
    if (x.size() < m)
        qDebug() << "time_series should have more elements than the AR order is";

    // INITIALIZE Ak
    vector<long double> Ak( m + 1, 0.0 );
    Ak[ 0 ] = 1.0;

    // INITIALIZE f and b
    vector<long double> f; // was double
    f.resize(x.size());
    for ( int i = 0; i < x.size(); i++ ) f[i] = x[i];
//    vector<long double> f( ldx );
    vector<long double> b( f ); // was double

    // INITIALIZE Dk
    long double Dk = 0.0; // was double
    for ( size_t j = 0; j <= N; j++ )
    {
        Dk += 2.0 * f[ j ] * f[ j ];
    }
    Dk -= f[ 0 ] * f[ 0 ] + b[ N ] * b[ N ];

    //// N is $#x-1 in C++ but $#x in perl
    //    my $Dk = sum map {
    //        2.0 * $f[$_] ** 2
    //    } 0 .. $#f;
    //    $Dk -= $f[0] ** 2 + $B[$#x] ** 2;

//    qDebug() << "Dk" << qStringFromLongDouble1(Dk);
    if ( isnan(Dk) )
    { qDebug() << "NAN at init"; }

    // BURG RECURSION
    for ( size_t k = 0; k < m; k++ )
    {
        // COMPUTE MU
        long double mu = 0.0;
        for ( size_t n = 0; n <= N - k - 1; n++ )
        {
            mu += f[ n + k + 1 ] * b[ n ];
        }

            mu *= -2.0 / Dk;
//            if ( isnan(Dk) )  { qDebug() << "k" << k; } assert

//            if (Dk!=0.0) {}
//        else qDebug() << "k" << k << "Dk==0" << qStringFromLongDouble1(Dk);

        //// N is $#x-1
        //# compute mu
        //my $mu = sum map {
        //    $f[$_ + $k + 1] * $B[$_]
        //} 0 .. $#x - $k - 1;
        //$mu *= -2.0 / $Dk;


        // UPDATE Ak
        for ( size_t n = 0; n <= ( k + 1 ) / 2; n++ )
        {
            long double t1 = Ak[ n ] + mu * Ak[ k + 1 - n ];
            long double t2 = Ak[ k + 1 - n ] + mu * Ak[ n ];
            Ak[ n ] = t1;
            Ak[ k + 1 - n ] = t2;
        }

        // UPDATE f and b
        for ( size_t n = 0; n <= N - k - 1; n++ )
        {
            long double t1 = f[ n + k + 1 ] + mu * b[ n ]; // were double
            long double t2 = b[ n ] + mu * f[ n + k + 1 ];
            f[ n + k + 1 ] = t1;
            b[ n ] = t2;
        }

        // UPDATE Dk
        Dk = ( 1.0 - mu * mu ) * Dk
                - f[ k + 1 ] * f[ k + 1 ]
                - b[ N - k - 1 ] * b[ N - k - 1 ];

    }
    // ASSIGN COEFFICIENTS
    coeffs.assign( ++Ak.begin(), Ak.end() );

    //    return $self->_set_coefficients([ @Ak[1 .. $#Ak] ]);

}

void BurgAlgorithm::predict( vector<long double> &coeffs, vector<double> &tail )
{
    size_t m = coeffs.size();
//    qDebug() << "tail.at(0)" << tail[0]*32768;
//    qDebug() << "tail.at(1)" << tail[1]*32768;
    tail.resize(m+tail.size());
//    qDebug() << "tail.at(m)" << tail[m]*32768;
//    qDebug() << "tail.at(...end...)" << tail[tail.size()-1]*32768;
//    qDebug() << "m" << m << "tail.size()" << tail.size();
    for ( size_t i = m; i < tail.size(); i++ )
    {
        tail[ i ] = 0.0;
        for ( size_t j = 0; j < m; j++ )
        {
            tail[ i ] -= coeffs[ j ] * tail[ i - 1 - j ];
        }
    }
}

//sub predict {
//    my ($self, $n) = @_;

//    my $coeffs = $self->coefficients;
//    my $m = $self->order;
//    $n ||= $m
//        if !$n || $n > $m;

//    my @predicted = @{ $self->series_tail };
//    for my $i ($m .. $m + $n) {
//        $predicted[$i] = -1.0 * sum map {
//            $coeffs->[$_] * $predicted[$i - 1 - $_]
//        } 0 .. $m - 1;
//    }

//    return [ @predicted[$m .. $#predicted] ];
//}


// Returns in vector coefficients calculated using Burg algorithm
// applied to the input source data x
void BurgAlgorithm::compute( vector<long double> &coeffs, const vector<double> &x )
{
//    // GET SIZE FROM INPUT VECTORS
//    size_t N = x.size() - 1;
//    size_t m = coeffs.size();
//    // INITIALIZE Ak
//    vector<double> Ak( m + 1, 0.0 );
//    Ak[ 0 ] = 1.0;
//    // INITIALIZE f and b
//    vector<double> f( x );
//    vector<double> b( x );
//    // INITIALIZE Dk
//    double Dk = 0.0;
//    for ( size_t j = 0; j <= N; j++ )
//    {
//        Dk += 2.0 * f[ j ] * f[ j ];
//    }
//    Dk -= f[ 0 ] * f[ 0 ] + b[ N ] * b[ N ];
//    // BURG RECURSION
//    for ( size_t k = 0; k < m; k++ )
//    {
//        // COMPUTE MU
//        double mu = 0.0;
//        for ( size_t n = 0; n <= N - k - 1; n++ )
//        {
//            mu += f[ n + k + 1 ] * b[ n ];
//        }
//        mu *= -2.0 / Dk;
//        // UPDATE Ak
//        for ( size_t n = 0; n <= ( k + 1 ) / 2; n++ )
//        {
//            double t1 = Ak[ n ] + mu * Ak[ k + 1 - n ];
//            double t2 = Ak[ k + 1 - n ] + mu * Ak[ n ];
//            Ak[ n ] = t1;
//            Ak[ k + 1 - n ] = t2;
//        }
//        // UPDATE f and b
//        for ( size_t n = 0; n <= N - k - 1; n++ )
//        {
//            double t1 = f[ n + k + 1 ] + mu * b[ n ];
//            double t2 = b[ n ] + mu * f[ n + k + 1 ];
//            f[ n + k + 1 ] = t1;
//            b[ n ] = t2;
//        }
//        // UPDATE Dk
//        Dk = ( 1.0 - mu * mu ) * Dk - f[ k + 1 ] * f[ k + 1 ] - b[ N - k - 1 ] * b[ N - k - 1 ];
//    }
//    // ASSIGN COEFFICIENTS
//    coeffs.assign( ++Ak.begin(), Ak.end() );
}
