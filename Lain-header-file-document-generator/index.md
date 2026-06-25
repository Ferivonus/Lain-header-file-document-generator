# Lain-header-file-document-generator

---

## Dosya: `.\test\file_sistem.h`

### `[FUNCTION]` `#define MAX(a,b) ((a) > (b) ? (a) : (b))`

**Ozet:**  Iki degerden buyuk olani donen makro tanimi.

**Detaylar:** Bu makro girilen <i>a</i> ve <i>b</i> parametrelerini karsilastirarak buyuk olani secer.

---

### `[TYPEDEF]` `typedef unsigned int UINT32`

**Ozet:**  32-bitlik isaretsiz tamsayi veri tipi tanimi.

---

### `[VARIABLE]` `int cihaz_durumu`

**Ozet:**  Sistemin anlik calisma modunu ve hata durumunu tutan degisken.

> **Uyari:**  Thread-safe degildir! Es zamanli bellek erisimlerinde veri tutarsizligina neden olabilir.

**Ayrica Bakiniz:** ` log_yaz()`

---

### `[FUNCTION]` `int topla(int sayi1, int sayi2)`

**Detaylar:** Iki sayiyi toplar ve sonucu geri dondurur.

#### Parametreler:
| Parametre | Aciklama |
| :--- | :--- |
| **sayi1** | Isleme girecek ilk tamsayi degeri. |
| **sayi2** | Isleme girecek ikinci tamsayi degeri. |

**Ayrica Bakiniz:** ` MAX(a,b)`

---

### `[FUNCTION]` `void log_yaz(const char *mesaj, int seviye)`

**Ozet:**  Konsola veya harici dosyaya gecerli zaman damgasiyla log kaydi duser.

#### Parametreler:
| Parametre | Aciklama |
| :--- | :--- |
| **mesaj** | Log dosyasına yazilacak olan ana metin icerigi. |
| **seviye** | Log öncelik derecesi (0: Bilgi, 1: Uyari, 2: Kritik Hata). |

> **Uyari:**  Gecersiz bir seviye girilirse sistem hata vermez, varsayilan olarak Bilgi kabul eder.

---

### `[FUNCTION]` `void sistemi_resetle()`

**Detaylar:** Bagli olan tum donanim birimlerini ve bellek tamponlarini sifirlar.

> **Uyari:**  Bu fonksiyon cagirildiginda kaydedilmemis tum anlik veriler kaybolacaktir.

---

