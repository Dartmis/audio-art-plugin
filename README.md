# Audio ART — native/VST port (v0.1, skeleton)

זהו שלב ראשון של הפיכת האנלייזר ל-VST3 ולתוכנה עצמאית (Standalone) למק ולווינדוס,
בעזרת GitHub Actions — בלי שום צורך בקומפיילר מקומי.

## מה יש כאן עכשיו (v0.1)

פורט ראשוני ב-JUCE/C++ שמדגים את התשתית ועובד end-to-end:

- מד Peak/RMS עם "הקפאה" וקליק-לאיפוס, בדיוק כמו ההתנהגות שסגרנו ב-HTML.
- LUFS Momentary / Short-term / Integrated (BS.1770 מקורב) עם warm-up עצמאי לכל אחד וקליק-לאיפוס.
- אנלייזר ספקטרום (FFT, 48 פסים לוגריתמיים).
- וקטורסקופ (Goniometer) בקו אחיד נקי, בלי glow — כמו הגרסה הסופית ב-HTML.
- בונה גם VST3 וגם Standalone מאותו קוד (JUCE).

**מה עדיין חסר** (v2 ואילך): רפרנס טראק + overlay, פריסטות/צבעים מהCustomize, True Peak, שכבת
ה-UI המלאה (עיצוב פאנלים, מרווחים) — אלה יתווספו בהדרגה, בדיוק כמו שעשינו ב-HTML.

## איך זה בונה את עצמו (GitHub Actions)

כל push ל-branch `main` מריץ שני runners מרוחקים של GitHub עצמו:
- **macos-14** — עם Xcode אמיתי מותקן.
- **windows-latest** — עם Visual Studio (MSBuild) אמיתי מותקן.

כל אחד מהם מקמפל את הקוד (JUCE מתחבר אוטומטית דרך CMake FetchContent), ומעלה **artifacts**
להורדה: `AudioART-macOS.zip` (מכיל `Audio ART.vst3` + `Audio ART.app`) ו-
`AudioART-Windows.zip` (מכיל `Audio ART.vst3` + `Audio ART.exe`).

### שלבים להרצה בפועל

1. ליצור ריפו חדש בגיטהאב (ציבורי או פרטי, לא משנה).
2. להעלות את כל התיקייה הזו לריפו (`git init`, `git add .`, `git commit`, `git push`).
3. בלשונית **Actions** בריפו — ה-workflow `Build Audio ART` יתחיל לרוץ אוטומטית.
   אם לא, אפשר להריץ אותו ידנית עם הכפתור "Run workflow".
4. אחרי כ-10-15 דקות (הבנייה הראשונה איטית כי JUCE מתקמפל מהתחלה), בעמוד ה-run
   יהיו שני קבצי zip להורדה תחת "Artifacts".

### התקנה בפועל

- **Mac**: לחלץ את ה-zip, להעביר את `Audio ART.vst3` לתיקיית
  `~/Library/Audio/Plug-Ins/VST3/`. בלי Apple Developer account (99$/שנה) ה-Gatekeeper
  יתריע בפעם הראשונה — פותרים בלחיצה ימנית → Open, או ב-System Settings → Privacy & Security.
  בלי זה אי אפשר להפיץ את זה בלי אזהרה לאף אחד אחר, אבל לשימוש אישי זה עובד מצוין.
- **Windows**: להעביר את `Audio ART.vst3` ל- `C:\Program Files\Common Files\VST3\`.
- ה-Standalone (`Audio ART.app` / `.exe`) רץ כתוכנה רגילה, בלי DAW.

## מבנה הקוד

```
Source/
  PluginProcessor.*     — ה"מנוע": מעבד אודיו, מחזיק את כל האנלייזרים
  PluginEditor.*         — ה-UI (ציור הפאנלים, קליק-לאיפוס)
  Meters.h                — Peak/RMS עם sample-and-hold
  LoudnessMeter.h         — LUFS M/S/I
  SpectrumAnalyzer.h      — FFT + פסים לוגריתמיים
  Goniometer.h            — באפר ל-וקטורסקופ
CMakeLists.txt            — מגדיר את הבנייה (מוריד JUCE אוטומטית)
.github/workflows/build.yml — ה-CI שבונה בפועל על שרתי מק/ווינדוס של גיטהאב
```
