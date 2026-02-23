# Projektisuunnitelma – FMI Säätiedot

## Projektin tavoite

Rakentaa web-sovellus, joka:
1. Hakee automaattisesti säätietoja (toteutuneet havainnot + ennusteet) FMI:n avoimesta datasta
2. Tallettaa tiedot aikasarjatietokantaan
3. Esittää tiedot selainpohjaisella käyttöliittymällä kaavioina ja taulukoina

---

## Toiminnalliset vaatimukset

### Tiedonkeruu
- [x] Hakee 10 minuutin välein uusimmat havainnot koko Suomesta (WFS multipointcoverage)
- [x] Hakee tunneittain Harmonie-ennusteen (~66h) konfiguroiduille paikkakunnille
- [x] Tallettaa asemien metatiedot (koordinaatit, nimi, maakunta)
- [x] Lokittaa jokaisen haun (onnistuminen/virhe, haettu määrä)

### Tiedon tallennus
- [x] Havainnot tallennetaan aikasarjatietokantaan (TimescaleDB hypertaulu)
- [x] Kaksoiskirjaukset estetään (ON CONFLICT DO NOTHING)
- [x] Tietokantamigraatiot Alembicillä

### API
- [x] REST-rajapinta havainnoille (aikaväli, resoluutio: raw/hourly/daily)
- [x] REST-rajapinta ennusteille (malli, tuntimäärä)
- [x] Asemalistaus ja yksittäinen asema
- [x] Yhteenveto (min/max/avg) ajanjaksolle
- [x] Ajastustöiden hallinta (status, manuaalinen käynnistys, loki)
- [x] Terveystarkistus (/health)

### Frontend
- [x] Asemavalinta pudotusvalikosta
- [x] Nykyisten olosuhteiden kortti (lämpötila, tuuli, kosteus, paine, sade)
- [x] Aikajanakuvaajat: lämpötila, sade/lumi, tuuli
- [x] Aikavälin valinta: 6h / 24h / 7 vrk / 30 vrk
- [x] Ennustetaulukko (Harmonie)
- [x] Automaattinen päivitys

---

## Ei-toiminnalliset vaatimukset

| Vaatimus | Tavoite |
|----------|---------|
| Suorituskyky | API-vastaus < 500 ms normaalikäytössä |
| Luotettavuus | Hakuvirheet kirjataan, seuraava haku jatkuu normaalisti |
| Skaalautuvuus | TimescaleDB hypertaulut + kompressio pitkälle historiatiedolle |
| Ylläpidettävyys | Docker Compose, ympäristömuuttujat, Alembic-migraatiot |
| Tietoturva | Ei käyttäjätunnistusta (sisäinen työkalu), CORS konfiguroitavissa |
| Lisenssi | FMI avoin data: CC BY 4.0 |

---

## Käyttötapaukset

### UC1: Käyttäjä näkee nykyiset sääolot
1. Käyttäjä avaa selaimen osoitteessa http://localhost:3000
2. Järjestelmä näyttää oletusaseman (Helsinki Kaisaniemi) viimeisimmät tiedot
3. Käyttäjä voi vaihtaa asemaa pudotusvalikosta
4. Näyttö päivittyy minuutin välein automaattisesti

### UC2: Käyttäjä tarkastelee historiatietoja
1. Käyttäjä valitsee "Havainnot"-välilehden
2. Valitsee aikavälin (6h/24h/7vrk/30vrk)
3. Järjestelmä näyttää lämpötila-, sade- ja tuulikaaviot

### UC3: Käyttäjä tarkastelee ennustetta
1. Käyttäjä valitsee "Ennuste"-välilehden
2. Järjestelmä näyttää Harmonie-ennusteen taulukkona (~66h)

### UC4: Järjestelmä hakee tietoja FMI:ltä
1. APScheduler käynnistää haun 10 min välein
2. FMI WFS -pyyntö koko Suomen bounding boxille
3. Tiedot parsitaan ja tallennetaan tietokantaan
4. Tulos kirjataan fetch_log-tauluun

---

## Toteutussuunnitelma

### Vaihe 1 – Infrastruktuuri
- [x] Docker Compose (PostgreSQL+TimescaleDB, backend, frontend)
- [x] Alembic-migraatiot (taulut, hypertaulut)
- [x] FastAPI-runko, terveystarkistus
- [x] Ympäristömuuttujien hallinta

### Vaihe 2 – FMI-integraatio ja backend
- [x] FMI WFS -asiakas (fmiopendata-kirjasto)
- [x] XML-parserointi → tietokantarivit
- [x] APScheduler-ajastustyöt
- [x] SQLAlchemy-palvelukerros (upsert, kyselyt)
- [x] REST API -päätepisteet

### Vaihe 3 – Frontend-pohja
- [x] Vite + React + TypeScript + Tailwind
- [x] API-asiakas (axios + SWR)
- [x] Asemavalinta, nykyiset olosuhteet (WeatherCard)

### Vaihe 4 – Kaaviot ja viimeistely
- [x] Recharts: lämpötila-, sade-, tuulikaaviot
- [x] Ennustetaulukko
- [x] Aikavälin valinta
- [x] Tumma teema, responsiivinen asettelu

---

## FMI avoimen datan käyttö

### Palvelu
- **URL**: https://opendata.fmi.fi/wfs
- **Protokolla**: OGC WFS 2.0
- **Rekisteröinti**: Ei vaadita (muutos 2023)
- **Lisenssi**: Creative Commons Attribution 4.0 (CC BY 4.0)
- **Attribuutio**: "Lähde: Ilmatieteen laitos"

### Käytetyt WFS-kyselyt
| Kysely | Käyttötarkoitus | Väli |
|--------|----------------|------|
| `fmi::observations::weather::multipointcoverage` | Kaikki asemat, Suomen bbox | 10 min |
| `fmi::forecast::harmonie::surface::point::multipointcoverage` | Pistepiste-ennuste | 60 min |

### Hyvät käytännöt
- Käytetään 20 min päällekkäistä ikkunaa havaintohaussa (ei aukkoja)
- `timestep=10` parametri rajoittaa datamäärää
- Backfill-hauissa 2 sekunnin viive pyyntöjen välillä
- `max_instances=1` estää samanaikaiset haut

---

## Riskit ja haasteet

| Riski | Todennäköisyys | Vaikutus | Hallinta |
|-------|---------------|---------|---------|
| FMI-palvelu tilapäisesti poissa | Matala | Matala | Virheloki, seuraava haku jatkaa |
| fmiopendata-kirjaston parametrinimet muuttuvat | Matala | Korkea | Parsiminen graceful, logitetaan tuntematon data |
| TimescaleDB ei saatavilla | Matala | Matala | Fallback plain PostgreSQL, time_bucket-virhe käsitellään |
| Suuri datamäärä hidastaa kyselyitä | Korkea (pitkällä aikavälillä) | Matala | Hypertaulut + kompressio + aggregaattikyselyt |
| Muistivuoto pitkäaikaisessa ajossa | Matala | Korkea | `expire_on_commit=False`, async session suljetaan aina |
