#ifndef PLATFORMS
#define PLATFORMS

#include <QHash>
#include <QList>
#include <QPair>
#include <QString>

namespace Library {

    enum class Platforms {
        Unknown = 0,
        GB = 1,
        GBC = 2,

        NES = 4,
        SNES = 5,
        N64 = 6,
        PSX = 7,

        GENESIS = 8,

        WII = 9,
        GAMECUBE = 10,
        MAX = 11,
    };

    QString platformtoString( const Platforms &value );

    static const QHash<QString, QString> platformMap {
        { "sfc" , "Super Nintendo"  }, // BSNES
        { "smc" , "Super Nintendo"  }, // BSNES
        { "bml" , "Super Nintendo"  }, // BSNES

        { "nes" , "Nintendo Entertainment System"  },  // Nestopia
        { "fds" , "Nintendo Entertainment System"  },  // Nestopia
        { "unif" , "Nintendo Entertainment System"  }, // Fceumm

        { "n64" , "Nintendo 64"  }, // Mupen64plus
        { "z64" , "Nintendo 64"  }, // Mupen64plus
        { "v64" , "Nintendo 64"  }, // Mupen64plus

        { "gba" , "Game Boy Advance"  }, // MGBA
        { "agb" , "Game Boy Advance"  }, // MGBA
        { "gbz" , "Game Boy Advance"  }, // MGBA

    };


    class BinaryHeaderOffset {
        public:
            BinaryHeaderOffset( int _offset, int _length );

            int offset;
            int length;
    };

    // These offset values are obtained book looking at rom hacking sources.
    // Just google something like 'Sega Genesis rom header' and you'll
    // find some good sources.
    const QHash<QString, QList<BinaryHeaderOffset>> headerOffsets    {
        {
            "bin", {
                BinaryHeaderOffset( 37664, 11 ),       // PlayStation Header
                BinaryHeaderOffset( 256, 15 ),         // Sega Genesis Header
            }
        },

        {
            "iso", {
                BinaryHeaderOffset( 24 , 4 ),             // Wii / GameCube Header
            }
        }
    };

    QString platformForHeaderString( const QByteArray &headerString );

}
#endif // PLATFORMS

