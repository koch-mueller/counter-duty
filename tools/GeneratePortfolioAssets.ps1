param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$source = @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public static class PortfolioAssetGenerator
{
    private const int SampleRate = 44100;

    private static short ToSample(double value)
    {
        value = Math.Max(-1.0, Math.Min(1.0, value));
        return (short)Math.Round(value * short.MaxValue);
    }

    private static void WriteWave(string path, double duration, Func<double, double> sample)
    {
        int sampleCount = (int)Math.Round(duration * SampleRate);
        Directory.CreateDirectory(Path.GetDirectoryName(path));

        using (FileStream stream = File.Create(path))
        using (BinaryWriter writer = new BinaryWriter(stream))
        {
            int dataSize = sampleCount * sizeof(short);
            writer.Write(new char[] {'R', 'I', 'F', 'F'});
            writer.Write(36 + dataSize);
            writer.Write(new char[] {'W', 'A', 'V', 'E'});
            writer.Write(new char[] {'f', 'm', 't', ' '});
            writer.Write(16);
            writer.Write((short)1);
            writer.Write((short)1);
            writer.Write(SampleRate);
            writer.Write(SampleRate * sizeof(short));
            writer.Write((short)sizeof(short));
            writer.Write((short)16);
            writer.Write(new char[] {'d', 'a', 't', 'a'});
            writer.Write(dataSize);

            for (int index = 0; index < sampleCount; ++index)
            {
                writer.Write(ToSample(sample(index / (double)SampleRate)));
            }
        }
    }

    private static double Sine(double frequency, double time)
    {
        return Math.Sin(2.0 * Math.PI * frequency * time);
    }

    public static void GenerateAudio(string projectRoot)
    {
        string soundRoot = Path.Combine(projectRoot, "assets", "sounds");

        double[][] chords =
        {
            new double[] {130.81, 164.81, 196.00, 246.94},
            new double[] {110.00, 130.81, 164.81, 196.00},
            new double[] {87.31, 130.81, 174.61, 220.00},
            new double[] {98.00, 146.83, 196.00, 246.94}
        };

        WriteWave(Path.Combine(soundRoot, "music", "music.wav"), 24.0, time =>
        {
            int chordIndex = ((int)(time / 6.0)) % chords.Length;
            double localTime = time % 6.0;
            double fade = Math.Min(1.0, Math.Min(localTime / 0.45, (6.0 - localTime) / 0.45));
            double value = 0.0;

            for (int note = 0; note < chords[chordIndex].Length; ++note)
            {
                double frequency = chords[chordIndex][note];
                value += Sine(frequency, time) * 0.055;
                value += Sine(frequency * 2.0, time) * 0.012;
            }

            double pulse = 0.5 + 0.5 * Math.Sin(2.0 * Math.PI * 0.25 * time);
            return value * fade * (0.72 + 0.12 * pulse);
        });

        double[] introNotes = {523.25, 659.25, 783.99, 1046.50};
        WriteWave(Path.Combine(soundRoot, "intro.wav"), 2.4, time =>
        {
            double value = 0.0;

            for (int note = 0; note < introNotes.Length; ++note)
            {
                double start = note * 0.24;
                double age = time - start;

                if (age < 0.0)
                {
                    continue;
                }

                double envelope = Math.Exp(-2.7 * age);
                value += Sine(introNotes[note], age) * envelope * 0.18;
                value += Sine(introNotes[note] * 2.01, age) * envelope * 0.035;
            }

            return value;
        });

        WriteWave(Path.Combine(soundRoot, "scanner.wav"), 0.22, time =>
        {
            double frequency = time < 0.11 ? 1046.50 : 1318.51;
            double envelope = Math.Min(1.0, time / 0.008) * Math.Max(0.0, (0.22 - time) / 0.04);
            return Sine(frequency, time) * envelope * 0.28;
        });
    }

    public static void GenerateBarcode(string projectRoot)
    {
        string path = Path.Combine(projectRoot, "assets", "materials", "barcode", "barcode_diffuse.png");
        Directory.CreateDirectory(Path.GetDirectoryName(path));

        using (Bitmap bitmap = new Bitmap(512, 256, PixelFormat.Format32bppArgb))
        using (Graphics graphics = Graphics.FromImage(bitmap))
        {
            graphics.Clear(Color.FromArgb(255, 244, 240, 228));

            int[] pattern = {2, 1, 2, 2, 1, 3, 1, 1, 3, 2, 2, 1, 1, 3, 2, 1, 2, 3,
                             1, 2, 1, 3, 2, 2, 1, 1, 2, 3, 3, 1, 1, 2, 2, 3, 1, 2};
            int x = 42;
            bool black = true;

            using (Brush ink = new SolidBrush(Color.FromArgb(255, 22, 24, 25)))
            {
                for (int index = 0; index < pattern.Length; ++index)
                {
                    int width = pattern[index] * 5;
                    if (black)
                    {
                        int height = (index % 7 == 0 || index > pattern.Length - 4) ? 170 : 150;
                        graphics.FillRectangle(ink, x, 32, width, height);
                    }

                    x += width;
                    black = !black;
                }

                using (Font font = new Font(FontFamily.GenericMonospace, 22.0f, FontStyle.Regular))
                {
                    graphics.DrawString("4 048462 091742", font, ink, new PointF(126.0f, 205.0f));
                }
            }

            bitmap.Save(path, ImageFormat.Png);
        }
    }
}
'@

$drawingAssembly = Join-Path $PSHOME 'System.Drawing.Common.dll'
$drawingPrimitivesAssembly = Join-Path $PSHOME 'System.Drawing.Primitives.dll'
$gdiAssembly = Join-Path $PSHOME 'System.Private.Windows.GdiPlus.dll'
$windowsCoreAssembly = Join-Path $PSHOME 'System.Private.Windows.Core.dll'

Add-Type -Path $drawingAssembly -ErrorAction Stop
Add-Type -TypeDefinition $source `
    -ReferencedAssemblies @($drawingAssembly, $drawingPrimitivesAssembly, $gdiAssembly, $windowsCoreAssembly) `
    -ErrorAction Stop
[PortfolioAssetGenerator]::GenerateAudio($ProjectRoot)
[PortfolioAssetGenerator]::GenerateBarcode($ProjectRoot)

Write-Host "Portfolio audio and barcode assets generated."
