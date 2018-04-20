ROOT = '../../../../'
SYSTEM = 'System'
COMPONENT = 'Component'
SINGLE_COMPONENT = 'SingleComponent'
REFLECTION_REGISTER = 'ReflectionRegister'
FILE = 'Name'


config = {
    'Network': {
        SYSTEM: 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Systems',
        COMPONENT: 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components',
        SINGLE_COMPONENT: 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components/SingleComponents',
        REFLECTION_REGISTER: {
            FILE: 'Modules/NetworkCore/Sources/NetworkCore/Private/NetworkCoreModule.cpp',
            COMPONENT: 'NetworkCore/Scene3D/Components',
            SINGLE_COMPONENT: 'NetworkCore/Scene3D/Components/SingleComponents',
            SYSTEM: 'NetworkCore/Scene3D/Systems',
        }

    },
    'Game': {
        SYSTEM: 'Programs/NetworkTestGame/Shared/Sources/Systems',
        COMPONENT: 'Programs/NetworkTestGame/Shared/Sources/Components',
        SINGLE_COMPONENT: 'Programs/NetworkTestGame/Shared/Sources/Components/SingleComponents',
        REFLECTION_REGISTER: {
            FILE: 'Programs/NetworkTestGame/Shared/Sources/Game.cpp',
            COMPONENT: 'Components',
            SINGLE_COMPONENT: 'Components/SingleComponents',
            SYSTEM: 'Systems',
        }
    },
}