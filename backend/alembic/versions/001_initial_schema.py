"""Initial schema

Revision ID: 001
Revises:
Create Date: 2026-02-20
"""
from alembic import op
import sqlalchemy as sa
from sqlalchemy.dialects import postgresql

revision = "001"
down_revision = None
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.create_table(
        "stations",
        sa.Column("fmisid", sa.Integer(), primary_key=True),
        sa.Column("name", sa.Text(), nullable=False),
        sa.Column("region", sa.Text()),
        sa.Column("country", sa.Text(), server_default="Finland"),
        sa.Column("latitude", sa.Float(), nullable=False),
        sa.Column("longitude", sa.Float(), nullable=False),
        sa.Column("elevation", sa.Float()),
        sa.Column("station_type", sa.Text()),
        sa.Column("is_active", sa.Boolean(), server_default="true"),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )
    op.create_index("idx_stations_latlon", "stations", ["latitude", "longitude"])

    op.create_table(
        "observations",
        sa.Column("time", sa.DateTime(timezone=True), nullable=False),
        sa.Column("fmisid", sa.Integer(), nullable=False),
        sa.Column("temperature", sa.Float()),
        sa.Column("dew_point", sa.Float()),
        sa.Column("humidity", sa.Float()),
        sa.Column("wind_speed", sa.Float()),
        sa.Column("wind_gust", sa.Float()),
        sa.Column("wind_direction", sa.SmallInteger()),
        sa.Column("precipitation_1h", sa.Float()),
        sa.Column("precip_intensity", sa.Float()),
        sa.Column("snow_depth", sa.Float()),
        sa.Column("pressure", sa.Float()),
        sa.Column("visibility", sa.Integer()),
        sa.Column("cloud_cover", sa.SmallInteger()),
        sa.Column("weather_code", sa.SmallInteger()),
        sa.PrimaryKeyConstraint("time", "fmisid"),
    )
    op.create_index("idx_obs_fmisid_time", "observations", ["fmisid", sa.text("time DESC")])

    # Try to create TimescaleDB hypertable (requires TimescaleDB extension)
    try:
        op.execute("SELECT create_hypertable('observations', 'time', chunk_time_interval => INTERVAL '1 week', if_not_exists => TRUE)")
    except Exception:
        pass  # Plain PostgreSQL without TimescaleDB

    op.create_table(
        "forecasts",
        sa.Column("id", sa.BigInteger(), primary_key=True, autoincrement=True),
        sa.Column("fetched_at", sa.DateTime(timezone=True), nullable=False),
        sa.Column("valid_time", sa.DateTime(timezone=True), nullable=False),
        sa.Column("fmisid", sa.Integer()),
        sa.Column("place_name", sa.Text()),
        sa.Column("latitude", sa.Float()),
        sa.Column("longitude", sa.Float()),
        sa.Column("model", sa.Text(), nullable=False),
        sa.Column("temperature", sa.Float()),
        sa.Column("wind_speed", sa.Float()),
        sa.Column("wind_direction", sa.SmallInteger()),
        sa.Column("wind_gust", sa.Float()),
        sa.Column("precipitation_1h", sa.Float()),
        sa.Column("humidity", sa.Float()),
        sa.Column("pressure", sa.Float()),
        sa.Column("cloud_cover", sa.Float()),
        sa.Column("dew_point", sa.Float()),
        sa.Column("weather_symbol", sa.SmallInteger()),
    )
    op.create_index("idx_forecasts_model_time", "forecasts", ["model", sa.text("valid_time DESC")])
    op.create_index("idx_forecasts_fmisid_time", "forecasts", ["fmisid", sa.text("valid_time DESC")])

    op.create_table(
        "fetch_log",
        sa.Column("id", sa.BigInteger(), primary_key=True, autoincrement=True),
        sa.Column("started_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("finished_at", sa.DateTime(timezone=True)),
        sa.Column("job_type", sa.Text(), nullable=False),
        sa.Column("status", sa.Text(), nullable=False),
        sa.Column("records_fetched", sa.Integer(), server_default="0"),
        sa.Column("error_message", sa.Text()),
        sa.Column("query_params", postgresql.JSONB()),
    )


def downgrade() -> None:
    op.drop_table("fetch_log")
    op.drop_table("forecasts")
    op.drop_table("observations")
    op.drop_table("stations")
