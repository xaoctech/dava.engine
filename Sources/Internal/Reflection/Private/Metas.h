#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/EnumMap.h"
#include "Base/GlobalEnum.h"

namespace DAVA
{
namespace Metas
{
/** Defines that Reflected Field can't be changed */
class ReadOnly
{
};

/** Mark field as invisible in property panel */
class HiddenField
{
};

class DeveloperModeOnly
{
};

class DisplayName
{
public:
    DisplayName(const String& displayName);
    const String displayName;
};

/** Hint that property can be edited as slider */
class Slider
{
};

/**
    Defines valid range of value
    Control will try to cast minValue and maxValue to control specific type T
    If some of bound couldn't be casted to T, this bound will be equal std::numeric_limits<T>::min\max.
*/
class Range
{
public:
    Range(const Any& minValue, const Any& maxValue, const Any& step);
    const Any minValue;
    const Any maxValue;
    const Any step;
};

/**
    Specifies count of signs in fraction part of float number for editing
*/
class FloatNumberAccuracy
{
public:
    FloatNumberAccuracy(uint32 accuracy);
    const uint32 accuracy;
};

/**
 Specifies maximum count of characters in text for editing
 */
class MaxLength
{
public:
    MaxLength(uint32 length);
    const uint32 length;
};

/** Validation result */
struct ValidationResult
{
    /** This enum type defines the state in which a validated value can exist */
    enum class eState
    {
        /** Inputted value isn't valid and should be discarded */
        Invalid,
        /**
            Inputted value can be valid. For example value 4 invalid for Range(10, 99)
            but we should allow user continue editing because value 40 is valid
        */
        Intermediate,
        /** Inputted value is completely valid */
        Valid
    };

    /**
        \anchor validator_state
        Current state of validator
    */
    eState state = eState::Invalid;
    /**
        Validator can change value that user inputted
        Control will use this value only if \ref validator_state "state" equal Valid or Intermediate
    */
    Any fixedValue;
    /**
        Hint text for user, that describe why inputted value isn't valid
        Control use this only if \ref validator_state "state" equal Invalid
    */
    String message;
};

using TValidationFn = ValidationResult (*)(const Any& value, const Any& prevValue);

/** Validator for Reflected Field's value */
class Validator
{
public:
    Validator(const TValidationFn& fn);
    virtual ~Validator() = default;

    /**
        Validate value
        \arg \c value Inputted value by user
        \arg \c current value of Reflected Field
    */
    virtual ValidationResult Validate(const Any& value, const Any& prevValue) const;

private:
    TValidationFn fn;
};

/** Base class for all mate Enum types */
class Enum
{
public:
    virtual ~Enum() = default;
    /** Returns EnumMap that describe values of Enum */
    virtual const EnumMap* GetEnumMap() const = 0;
};

template <typename T>
class EnumT : public Enum
{
public:
    const EnumMap* GetEnumMap() const override;
};

template <typename T>
inline const EnumMap* EnumT<T>::GetEnumMap() const
{
    return GlobalEnumMap<T>::Instance();
}

/** Base class for all mate Enum types */
class Flags
{
public:
    virtual ~Flags() = default;
    /** Returns EnumMap that describe values of bitfield Enum */
    virtual const EnumMap* GetFlagsMap() const = 0;
};

template <typename T>
class FlagsT : public Flags
{
public:
    const EnumMap* GetFlagsMap() const override;
};

template <typename T>
inline const EnumMap* FlagsT<T>::GetFlagsMap() const
{
    return GlobalEnumMap<T>::Instance();
}

/** Defines that value of Reflected Field should be File */
class File
{
public:
    File(const String& filters, const String& dlgTitle = String("Open File"));
    virtual ~File() = default;

    virtual String GetDefaultPath() const;
    virtual String GetRootDirectory() const;

    const String filters;
    const String dlgTitle;
};

/** Defines that value of Reflected Field should be Directory */
class Directory
{
};

/** Defines logical group of set of Reflected Fields under the same name */
class Group
{
public:
    /** \arg \c groupName name of logical group */
    Group(const char* groupName);
    const char* groupName;
};

using TValueDescriptorFn = String (*)(const Any&);

/** Defines function that can provide string representation of value. */
class ValueDescription
{
public:
    ValueDescription(const TValueDescriptorFn& fn);

    String GetDescription(const Any& v) const;

private:
    TValueDescriptorFn fn;
};

/**
    We think about some types like about base types: Vector2, Vector3, Vector4, Color, Rect etc
    But in real this types are complex and have fields. For example Vector3 comprises the following fields: X, Y, Z
    This meta mark field of "BaseType" as "field to edit". As a reaction there will be created separate sub-editor
    for each field that marked by this meta
*/
class SubProperty
{
};

/**
    Says that value can be changed at some unpredictable moment and 
    Reflection's client should update value as often as possible
*/
class FrequentlyChangedValue
{
};

/** Type that derived from Component and marked by this Meta can't be created in PropertyPanel */
class CantBeCreatedManualyComponent
{
};

/** Type that derived from Component and marked by this Meta can't be deleted in PropertyPanel */
class CantBeDeletedManualyComponent
{
};

/** Says that type derived from Component and marked by this Meta can't be exported */
class NonExportableComponent
{
};

/** Says that type derived from Component and marked by this Meta can't be serialized */
class NonSerializableComponent
{
};

/** Indicate field in current type, that will return tooltip */
class Tooltip
{
public:
    Tooltip(const String& tooltipFieldName);
    String tooltipFieldName;
};

enum class Privacy : uint8
{
    SERVER_ONLY,
    PRIVATE,
    TEAM_ONLY,
    PUBLIC,
    AS_PARENT,
};

/** Components or field are simulated */
class Replicable
{
public:
    Replicable();
    Replicable(Privacy privacy);

    Privacy GetMergedPrivacy(Privacy parentPrivacy) const;

    mutable Privacy privacy;
};

/** Indicate that color's components should be edited as int*/
class IntColor
{
};

/** Marks UIComponent type as multiples instances per UIControl */
class Multiple
{
};

//// Network packing meta ////////////////////////////////////////////////

struct QuaternionNetPacking
{
    QuaternionNetPacking(uint32 bitsPerComponent);

    const uint32 bitsPerComponent;
};

struct FloatNetPacking
{
    struct Layout
    {
        float32 halfRange;
        float32 precision;
        uint32 intBits;
        uint32 fracBits;
    };

    FloatNetPacking(float32 rangeFull, float32 precisionFull, float32 rangeDelta, float32 precisionDelta);
    FloatNetPacking(float32 range, float32 precision);

    const Layout layoutFull;
    const Layout layoutDelta;

    static Layout ComputeLayout(float32 range, float32 precision);
};

struct Vector3NetPacking
{
    Vector3NetPacking(const FloatNetPacking componentXY, FloatNetPacking componentZ)
        : componentXY(componentXY)
        , componentZ(componentZ)
    {
    }
    const FloatNetPacking componentXY;
    const FloatNetPacking componentZ;
};

struct IntegerNetPacking
{
    IntegerNetPacking(int32 rangeMin, int32 rangeMax)
        : rangeMin(rangeMin)
        , rangeMax(rangeMax)
    {
    }
    const int32 rangeMin;
    const int32 rangeMax;
};

//////////////////////////////////////////////////////////////////////////
struct ComparePrecision
{
    ComparePrecision(float32 precision)
        : precision(precision)
    {
    }
    const float32 precision;
};

struct FloatQuantizeParam
{
    FloatQuantizeParam(float32 fullRange, float32 deltaRange, float32 precision)
        : fullRange(fullRange)
        , deltaRange(deltaRange)
        , precision(precision)
    {
    }
    FloatQuantizeParam(float32 fullRange, float32 precision)
        : fullRange(fullRange)
        , deltaRange(fullRange)
        , precision(precision)
    {
    }
    const float32 fullRange;
    const float32 deltaRange;
    const float32 precision;
};

struct QuaternionQuantizeParam
{
    QuaternionQuantizeParam(float32 precision)
        : precision(precision)
    {
    }
    const float32 precision;
};

struct IntCompressParam
{
    IntCompressParam(uint32 fullRange, uint32 deltaRange)
        : fullRange(fullRange)
        , deltaRange(deltaRange)
    {
    }
    IntCompressParam(uint32 fullRange)
        : fullRange(fullRange)
        , deltaRange(fullRange)
    {
    }
    const uint32 fullRange;
    const uint32 deltaRange;
};

struct Int64CompressParam
{
    Int64CompressParam(uint64 fullRange, uint64 deltaRange)
        : fullRange(fullRange)
        , deltaRange(deltaRange)
    {
    }
    Int64CompressParam(uint64 fullRange)
        : fullRange(fullRange)
        , deltaRange(fullRange)
    {
    }
    const uint64 fullRange;
    const uint64 deltaRange;
};

class Tags
{
public:
    template <typename... Args>
    Tags(Args&&... args)
    {
        tags = { FastName(args)... };
    }

    Vector<FastName> tags;
};

namespace SystemProcess
{
enum class Type
{
    NORMAL = 0, //!< normal process type. If process should be executed once per frame, choose this group. `Process` should override `SceneSystem::Process`.
    FIXED //!< fixed process type. If process should be executed once per fixed update time, choose this group. `ProcessFixed` should override `SceneSystem::ProcessFixed`.
};

enum class Group
{
    ENGINE_BEGIN = 0, //!< first part of base engine processes/fixed processes. Do not use this group for gameplay processes/fixed processes.
    GAMEPLAY_BEGIN, //!< gameplay processes/fixed processes. If gameplay fixed process should be executed before physics, choose this gorup.
    ENGINE_PHYSICS, //!< physics fixed process. Do not use this group, unless you know what this group for.
    GAMEPLAY_END, //!< gameplay processes/fixed processes. If gameplay fixed process should be executed after physics, choose this gorup.
    ENGINE_END //!< last part of base engine processes/fixed processes. Do not use this group for gameplay processes/fixed processes.
};

class SystemProcess
{
public:
    SystemProcess(Group group, Type type, float32 order);

    Group group;
    Type type;
    float32 order;
};

} // namespace SystemProcess

} // namespace Metas
} // namespace DAVA
