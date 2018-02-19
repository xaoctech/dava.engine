from enum import Enum

ROOT = '../../../../'
SYSTEM = 'System'
COMPONENT = 'Component'
SINGLE_COMPONENT = 'SingleComponent'

config = {
    'Network' : {
        SYSTEM : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Systems',
        COMPONENT : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components',
        SINGLE_COMPONENT : 'Modules/NetworkCore/Sources/NetworkCore/Scene3D/Components/SingleComponents'
    },
    'Game' : {
        SYSTEM : 'Programs/NetworkTestGame/Shared/Sources/Systems',
        COMPONENT : 'Programs/NetworkTestGame/Shared/Sources/Components',
        SINGLE_COMPONENT: 'Programs/NetworkTestGame/Shared/Sources/Components/SingleComponents'
    },
}