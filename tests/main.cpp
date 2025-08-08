//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.
*/
//*****************************************************************

/**
 * \file main.cpp
 * \author Chris Chafe
 * \date December 2024
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include "RegulatorTest.h"

int main(int argc, char **argv) {
    // Initialize Qt application first (required for RegulatorWorker)
    QCoreApplication app(argc, argv);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run tests
    int result = RUN_ALL_TESTS();
    
    return result;
}

