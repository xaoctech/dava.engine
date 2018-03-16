#pragma once

#include <utility>
#include <QtCore/QUuid>
#include <QString>

#include "make_unique.hpp"


namespace QtNodes
{

enum class PortType
{
  None = -1,

  In = 0,
  Out,

  Count
};

enum class PortKind
{
  None = -1,

  Data = 0,
  Execution,

   Count
};

static const size_t PortTypeCount = static_cast<size_t>(PortType::Count);
static const int INVALID = -1;

using PortIndex = int;

struct Port
{
  PortType type;

  PortIndex index;

  Port()
    : type(PortType::None)
    , index(INVALID)
  {}

  Port(PortType t, PortIndex i)
    : type(t)
    , index(i)
  {}

  bool
  indexIsValid() { return index != INVALID; }

  bool
  portTypeIsValid() { return type != PortType::None; }
};

//using PortAddress = std::pair<QUuid, PortIndex>;

inline
PortType
oppositePort(PortType port)
{
  PortType result = PortType::None;

  switch (port)
  {
    case PortType::In:
      result = PortType::Out;
      break;

    case PortType::Out:
      result = PortType::In;
      break;

    default:
      break;
  }

  return result;
}

inline
QString
PortPreffix(PortType port)
{
    if(port == PortType::In)
    {
        return "in_";
    }
    else if(port == PortType::Out)
    {
        return "out_";
    }

    return "";
}

inline
size_t
PortToIndex(PortType port)
{
    return static_cast<size_t>(port);
}

inline
PortType
IndexToPort(size_t port)
{
    return static_cast<PortType>(port);
}


}
