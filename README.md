# GG/SMS Header Writer

自作のゲームギアソフトを実機で動かすために必要なヘッダ情報を書き込むツールです。


```bash
gghead [-p PRODUCT_CODE]
       [-v VERSION_CODE]
       [-r REGION_CODE]
       /path/to/file.rom

Notes:
- PRODUCT_CODE = 00000~FFFFF (5-digit hexadecimal number)
- VERSION_CODE = 0~F (1-digit hexadecimal number)
- REGION_CODE = Valid codes include the following:
  - 3: SMS Japan
  - 4: SMS Export
  - 5: GG Japan <default>
  - 6: GG Export
  - 7: GG International
```

以下の情報を参考にしました。

[https://www.smspower.org/Development/ROMHeader](https://www.smspower.org/Development/ROMHeader)

