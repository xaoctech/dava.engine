from enum import Enum

ROOT = '../../../../'
SYSTEM = 'System'
COMPONENT = 'Component'
SINGLE_COMPONENT = 'SingleComponent'
REFLECTION_REGISTER = 'ReflectionRegister'


config = {
    'Network' : {
        SYSTEM : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Systems',
        COMPONENT : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components',
        SINGLE_COMPONENT : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components/SingleComponents',
        REFLECTION_REGISTER : 'Modules/NetworkCore/Sources/NetworkCore/Private/NetworkCoreModule.cpp',

    },
    'Game' : {
        SYSTEM : 'Programs/NetworkTestGame/Shared/Sources/Systems',
        COMPONENT : 'Programs/NetworkTestGame/Shared/Sources/Components',
        SINGLE_COMPONENT: 'Programs/NetworkTestGame/Shared/Sources/Components/SingleComponents',
        REFLECTION_REGISTER : 'Programs/NetworkTestGame/Shared/Sources/Game.cpp',
    },
}