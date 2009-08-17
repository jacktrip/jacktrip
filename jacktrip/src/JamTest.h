#ifndef __JAMTEST_H__
#define __JAMTEST_H__

/// \brief PlaceHolder Class
class JamTest : public JackTrip
{
public:
  JamTest(JackTrip::jacktripModeT JacktripMode = JackTrip::CLIENT) :
      JackTrip(JacktripMode)
  {}
  virtual ~JamTest() {}
};
#endif // __JAMTEST_H__
