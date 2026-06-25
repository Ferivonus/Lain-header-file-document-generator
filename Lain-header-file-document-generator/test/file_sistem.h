// lain-was-here
/*! \file sistem.h
    \brief Sistem yonetimi ve matematiksel makrolar iceren yardimci dosya.
    
    Bu dosya yazilim motorumuzun destekledigi tum gelismis etiketleri 
    ve bicimlendirmeleri test etmek amaciyla hazirlanmistir.
*/

/*! \def MAX(a,b)
    \brief Iki degerden buyuk olani donen makro tanimi.
    
    Bu makro girilen \a a ve \a b parametrelerini karsilastirarak buyuk olani secer.
*/
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/*! \var typedef unsigned int UINT32
    \brief 32-bitlik isaretsiz tamsayi veri tipi tanimi.
*/
typedef unsigned int UINT32;

/*! \var int cihaz_durumu
    \brief Sistemin anlik calisma modunu ve hata durumunu tutan degisken.
 
    \warning Thread-safe degildir! Es zamanli bellek erisimlerinde veri tutarsizligina neden olabilir.
    \see log_yaz()
*/
int cihaz_durumu;

/**
 * Iki sayiyi toplar ve sonucu geri dondurur.
 * @see MAX(a,b)
 * @param sayi1 Isleme girecek ilk tamsayi degeri.
 * @param sayi2 Isleme girecek ikinci tamsayi degeri.
 */
int topla(int sayi1, int sayi2);

/*! \fn void log_yaz(const char *mesaj, int seviye)
    \brief Konsola veya harici dosyaya gecerli zaman damgasiyla log kaydi duser.
 
    \param mesaj Log dosyasına yazilacak olan ana metin icerigi.
    \param seviye Log öncelik derecesi (0: Bilgi, 1: Uyari, 2: Kritik Hata).
    \warning Gecersiz bir seviye girilirse sistem hata vermez, varsayilan olarak Bilgi kabul eder.
*/
void log_yaz(const char *mesaj, int seviye);

/**
 * Bagli olan tum donanim birimlerini ve bellek tamponlarini sifirlar.
 * \warning Bu fonksiyon cagirildiginda kaydedilmemis tum anlik veriler kaybolacaktir.
 */
void sistemi_resetle();