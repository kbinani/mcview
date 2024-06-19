#version 120
#extension GL_EXT_gpu_shader4 : enable
varying vec2 textureCoordOut;
uniform sampler2D texture;
uniform float fade;
uniform int grassBlockId;
uniform int foliageBlockId;
uniform int netherrackBlockId;
uniform int waterBlockId;
uniform float waterOpticalDensity;
uniform bool waterTranslucent;
uniform int biomeBlend;
uniform bool enableBiome;
uniform float width;
uniform float height;
uniform int dimension;

uniform sampler2D north;
uniform sampler2D northEast;
uniform sampler2D east;
uniform sampler2D southEast;
uniform sampler2D south;
uniform sampler2D southWest;
uniform sampler2D west;
uniform sampler2D northWest;

uniform sampler2D palette;
uniform int paletteSize;
uniform int paletteType; // 0: mcview, 1: java, 2: bedrock
uniform int lightingType; // 1: top-left, 2: top

struct BlockInfo {
    float height;
    float waterDepth;
    int biomeId;
    int blockId;
    int biomeRadius;
};

float altitudeFromColor(vec4 color) {
    int a = int(color.a * 255.0);
    int r = int(color.r * 255.0);
    int h = (a << 1) + (0x1 & (r >> 7));
    return float(h) - 64.0;
}

BlockInfo pixelInfo(vec4 color) {
    // [v4 pixel info]
    // h:                      9bit
    // block/waterDepth flag:  1bit => 1: block, 0: waterDepth
    // block or waterDepth:   16bit
    // biome:                  3bit
    // biomeRadius:            3bit

    /*
     AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB
     hhhhhhhhhfwwwwwwwwwwwwwwwwbbbrrr : v4
     */

    int a = int(color.a * 255.0);
    int r = int(color.r * 255.0);
    int g = int(color.g * 255.0);
    int b = int(color.b * 255.0);

    float h = altitudeFromColor(color);

    int block;
    int depth;
    int blockOrDepth = ((0x3f & r) << 10) + (g << 2) + ((0xc0 & b) >> 6);
    if ((0x1 & (r >> 6)) == 0x1) {
      // block
      depth = 0;
      block = blockOrDepth;
    } else {
      // waterDepth
      depth = blockOrDepth;
      block = waterBlockId;
    }
    int biome = (b >> 3) & 0x7;
    int biomeRadius = b & 0x7;
    BlockInfo info;
    info.height = h;
    info.waterDepth = float(depth) / 127.0 * 255.0;
    info.biomeId = biome;
    info.blockId = block;
    info.biomeRadius = biomeRadius;
    return info;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;

    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 blend(vec4 bg, vec4 fg) {
    float a = fg.a + bg.a * (1.0 - fg.a);
    if (a == 0.0) {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
    float r = (fg.r * fg.a + bg.r * bg.a * (1.0 - fg.a)) / a;
    float g = (fg.g * fg.a + bg.g * bg.a * (1.0 - fg.a)) / a;
    float b = (fg.b * fg.a + bg.b * bg.a * (1.0 - fg.a)) / a;
    return vec4(r, g, b, a);
}

vec4 rgb(int r, int g, int b, int a) {
    return vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, float(a) / 255.0);
}

vec4 netherrack_colormap(float x) {
    float h = 1.0 / 360.0;
    float s = 64.0 / 100.0;
    float vmin = 16.0 / 100.0;
    float vmax = 50.0 / 100.0;
    float v = vmin + (vmax - vmin) * clamp(1.0 - x, 0.0, 1.0);
    return vec4(hsv2rgb(vec3(h, s, v)), 1.0);
}

float rand(float n) {
    return fract(sin(n) * 43758.5453123);
}

float noise(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 voidColor() {
    float s1 = (rand(gl_FragCoord.x) + rand(textureCoordOut.x)) * 0.5 * width;
    float s2 = (rand(gl_FragCoord.y) + rand(textureCoordOut.y)) * 0.5 * height;
    float x = noise(vec2(s1, s2));
    float y = noise(vec2(s2, s1));
    float bm1 = sqrt(-2 * log(x)) * cos(2 * 3.1415926 * y);
    float bm2 = sqrt(-2 * log(y)) * sin(2 * 3.1415926 * x);

    // H: average=0.733309; sigma=0.00610866; minmax=[0.666667, 0.766667]
    // S: average=0.448486; sigma=0.0703466; minmax=[0.0810811, 0.6]
    // B: average=0.0958848; sigma=0.0170297; minmax=[0.0117647, 0.145098]

    float h = 0.733309;

    float sAvg = 0.448486;
    float sSigma = 0.0703466;
    float sMin = 0.0810811;
    float sMax = 0.6;
    float s = clamp(sAvg + bm1 * sSigma, sMin, sMax);

    float vAvg = 0.0958848;
    float vSigma = 0.0170297;
    float vMin = 0.0117647;
    float vMax = 0.145098;

    float v = clamp(vAvg + bm2 * vSigma, vMin, vMax);
    vec3 c = hsv2rgb(vec3(h, s, v));
    return vec4(c.rgb, 1);
}

vec4 colormap(float x);

vec4 waterColorFromBiome(int biome);
vec4 foliageColorFromBiome(int biome);

vec4 colorFromBlockId(int blockId) {
    if (blockId == #{airBlockId}) {
        return vec4(0, 0, 0, 0);
    }
    int index = blockId - 1;
    int iy = index / paletteSize;
    int ix = index - iy * paletteSize;
    float fy = (iy + 0.5) / float(paletteSize);
    float fx = (ix + 0.5) / float(paletteSize);
    return texture2D(palette, vec2(fx, fy));
}

vec4 waterColor() {
    vec2 center = textureCoordOut;
    vec4 sumColor = vec4(0.0, 0.0, 0.0, 0.0);
    int count = 0;
    for (int dx = -biomeBlend; dx <= biomeBlend; dx++) {
        for (int dz = -biomeBlend; dz <= biomeBlend; dz++) {
            float x = center.x + float(dx) / 512.0;
            float y = center.y + float(dz) / 512.0;
            vec4 c;
            if (x < 0.0) {
                if (y < 0.0) {
                    c = texture2D(northWest, vec2(x + 1.0, y + 1.0));
                } else if (y < 1.0) {
                    c = texture2D(west, vec2(x + 1.0, y));
                } else {
                    c = texture2D(southWest, vec2(x + 1.0, y - 1.0));
                }
            } else if (x < 1.0) {
                if (y < 0.0) {
                    c = texture2D(north, vec2(x, y + 1.0));
                } else if (y < 1.0) {
                    c = texture2D(texture, vec2(x, y));
                } else {
                    c = texture2D(south, vec2(x, y - 1.0));
                }
            } else {
                if (y < 0.0) {
                    c = texture2D(northEast, vec2(x - 1.0, y + 1.0));
                } else if (y < 1.0) {
                    c = texture2D(east, vec2(x - 1.0, y));
                } else {
                    c = texture2D(southEast, vec2(x - 1.0, y - 1.0));
                }
            }
            BlockInfo info = pixelInfo(c);
            sumColor += waterColorFromBiome(info.biomeId);
            count++;
        }
    }
    return sumColor / float(count);
}

void main() {
    float alpha = fade;

    vec4 color = texture2D(texture, textureCoordOut);
    BlockInfo info = pixelInfo(color);

    float height = info.height;
    float waterDepth = info.waterDepth;
    int biomeId = info.biomeId;
    int blockId = info.blockId;
    bool isVoid = false;

    vec4 c;
    if (waterDepth > 0.0) {
        vec4 wc;
        if (enableBiome) {
            if (info.biomeRadius >= biomeBlend) {
                wc = waterColorFromBiome(info.biomeId);
            } else {
                wc = waterColor();
            }
        } else {
            wc = colorFromBlockId(waterBlockId);
        }
        if (waterTranslucent) {
            float intensity = pow(10.0, -waterOpticalDensity * waterDepth);
            c = vec4(wc.r * intensity, wc.g * intensity, wc.b* intensity, alpha);
        } else {
            c = wc;
        }
    } else if (blockId == foliageBlockId) {
        vec4 lc = foliageColorFromBiome(enableBiome ? biomeId : -1);
        c = vec4(lc.rgb, alpha);
    } else if (blockId == grassBlockId && paletteType == 0) {
        float v = (height - 63.0) / 384.0;
        vec4 g = colormap(v);
        c = vec4(g.rgb, alpha);
    } else if (blockId == netherrackBlockId) {
        float v = (height - 31.0) / (127.0 - 31.0);
        vec4 cc = netherrack_colormap(v);
        c = vec4(cc.rgb, alpha);
    } else if (blockId == 0) {
        if (dimension == 1) {
            c = voidColor();
            isVoid = true;
        } else {
            c = vec4(0.0, 0.0, 0.0, 0.0);
        }
    } else if (blockId == #{airBlockId} && dimension == 1) {
        c = voidColor();
        isVoid = true;
    } else {
        vec4 cc = colorFromBlockId(blockId);
        if (cc.a == 0.0) {
            c = cc;
        } else {
            c = vec4(cc.r, cc.g, cc.b, alpha);
        }
    }

    if (!isVoid && (waterDepth == 0.0 || (waterDepth > 0.0 && waterTranslucent))) {
        if (lightingType == 2) {
            float d = 1.0 / 512.0;
            float tx = textureCoordOut.x;
            float ty = textureCoordOut.y;
            vec4 northC;
            if (ty - d < 0.0) {
                northC = texture2D(north, vec2(tx, ty - d + 1.0));
            } else {
                northC = texture2D(texture, vec2(tx, ty - d));
            }
            float northH = altitudeFromColor(northC);
            float coeff = 220.0 / 255.0;
            if (northH > 0.0) {
                if (northH > height) coeff = 180.0 / 255.0;
                if (northH < height) coeff = 1;
            }
            c = vec4(c.rgb * coeff, c.a);
        } else {
            float heightScore = 0.0; // +: bright, -: dark
            float d = 1.0 / 512.0;
            float tx = textureCoordOut.x;
            float ty = textureCoordOut.y;
            vec4 northC;
            if (ty - d < 0.0) {
                northC = texture2D(north, vec2(tx, ty - d + 1.0));
            } else {
                northC = texture2D(texture, vec2(tx, ty - d));
            }
            vec4 westC;
            if (tx - d < 0.0) {
                westC = texture2D(west, vec2(tx - d + 1.0, ty));
            } else {
                westC = texture2D(texture, vec2(tx - d, ty));
            }
            float northH = altitudeFromColor(northC);
            float westH = altitudeFromColor(westC);
            if (northH > 0.0) {
                if (northH > height) heightScore--;
                if (northH < height) heightScore++;
            }
            if (westH > 0.0) {
                if (westH > height) heightScore--;
                if (westH < height) heightScore++;
            }

            if (heightScore > 0.0) {
                float coeff = 1.2;
                vec3 hsv = rgb2hsv(c.rgb);
                hsv.b = hsv.b * coeff;
                c = vec4(hsv2rgb(hsv).rgb, c.a);
            } else if (heightScore < 0.0) {
                float coeff = 0.8;
                vec3 hsv = rgb2hsv(c.rgb);
                hsv.b = hsv.b * coeff;
                c = vec4(hsv2rgb(hsv).rgb, c.a);
            }
        }
    }

    if (c.a == 0.0 && fade < 1.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 37.0 / 255.0 * (1.0 - fade));
    } else {
        if (fade < 1.0) {
            gl_FragColor = blend(c, vec4(0.0, 0.0, 0.0, 37.0 / 255.0 * (1.0 - fade)));
        } else {
            gl_FragColor = c;
        }
    }
}
